#!/usr/bin/env python3
"""Generate local, offline Mandarin narration assets with Kokoro v1.1-zh."""
import argparse
import hashlib
import json
import subprocess
import wave
from pathlib import Path
import numpy as np
import soundfile as sf
import onnxruntime as ort
from kokoro_onnx import Kokoro
from misaki import zh

STEPS=(7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767)
INDICES=(-1,-1,-1,-1,2,4,6,8)

def encode(samples):
    predictor=index=0; result=bytearray((len(samples)+1)//2)
    for i,sample in enumerate(samples):
        delta=int(sample)-predictor;code=8 if delta<0 else 0;delta=abs(delta);step=STEPS[index];difference=step>>3
        for bit,amount in ((4,step),(2,step>>1),(1,step>>2)):
            if delta>=amount:code|=bit;delta-=amount;difference+=amount
        predictor=max(-32768,min(32767,predictor+(-difference if code&8 else difference)))
        index=max(0,min(88,index+INDICES[code&7]));result[i//2]|=code<<((i&1)*4)
    return bytes(result)


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--model-dir',type=Path,required=True);parser.add_argument('--wav-dir',type=Path,required=True);args=parser.parse_args()
    root=Path(__file__).resolve().parents[1];out=root/'assets/voice';out.mkdir(exist_ok=True);args.wav_dir.mkdir(parents=True,exist_ok=True)
    rows=[json.loads('['+line.strip()[1:line.strip().rindex('}')]+']') for line in (root/'main/guide_data.c').read_text().splitlines() if line.strip().startswith('{')]
    assert len(rows)==22
    narration=json.loads((out/'narration.json').read_text())
    assert narration['voice']=='zf_001' and len(narration['clips'])==22
    ort.disable_telemetry_events();g2p=zh.ZHG2P(version='1.1')
    k=Kokoro(str(args.model_dir/'kokoro-v1.1-zh.onnx'),str(args.model_dir/'voices-v1.1-zh.bin'),vocab_config=str(args.model_dir/'config.json'))
    manifest={'model':'Kokoro v1.1-zh','voice':'zf_001','model_license':'Apache-2.0','sample_rate':16000,'encoding':'IMA/DVI 4-bit continuous low-nibble-first; initial predictor/index zero','models':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in args.model_dir.iterdir() if p.is_file()},'clips':[]}
    manifest['narration_sha256']=hashlib.sha256((out/'narration.json').read_bytes()).hexdigest()
    manifest['speed']=1.0
    manifest['processing']={'profile':'B-light-denoise-v1','filter':'highpass=f=85,lowpass=f=6200,afftdn=nr=8:nf=-42','trim_filter':'volume=0.11dB','stage_pcm16_between_filters':True,'dynamic_loudness':True,'linear_gain_db':0.0,'speaker_profile':'peak-aware-speech-v1','speaker_filter':'acompressor=threshold=0.06:ratio=3:attack=5:release=120:makeup=3:knee=2.5:detection=rms,alimiter=limit=0.35:level=false:attack=5:release=100:latency=true,loudnorm=I=-20:TP=-9:LRA=5'}
    declarations=['#include "guide_voice_assets.h"\n'];table=[]
    for id,row in enumerate(rows):
        name,_,intro,_,park,*_=row;script=narration['clips'][id]
        assert script['id']==id and script['name']==name and script['park']==('九寨沟','黄龙')[park]
        text=script['text']
        target=args.wav_dir/f'clip_{id:02}.wav';hq=args.wav_dir/f'clip_{id:02}-source.wav'
        cache=hq.with_suffix('.json')
        signature={'text':text,'voice':manifest['voice'],'models':manifest['models'],'speed':1.0}
        if not hq.exists() or not cache.exists() or json.loads(cache.read_text())!=signature:
            phonemes,_=g2p(text);assert '❓' not in phonemes
            audio,rate=k.create(phonemes,voice='zf_001',speed=1.0,is_phonemes=True);assert len(audio)>rate
            sf.write(hq,audio,rate,subtype='PCM_16')
            cache.write_text(json.dumps(signature,ensure_ascii=False,sort_keys=True)+'\n')
        subprocess.run(['ffmpeg','-v','error','-y','-i',str(hq),'-ar','16000','-ac','1','-c:a','pcm_s16le',str(target)],check=True)
        with wave.open(str(target)) as f:samples=np.frombuffer(f.readframes(f.getnframes()),dtype='<i2').astype(np.float64)
        peak=max(abs(samples));assert peak>100
        samples*=min(1.5,15000/peak)
        fade=min(640,len(samples)//4);samples[:fade]*=np.linspace(0,1,fade);samples[-fade:]*=np.linspace(1,0,fade)
        samples=np.concatenate([np.zeros(2400),samples,np.zeros(4000)]).astype('<i2')
        with wave.open(str(target),'wb') as f:f.setnchannels(1);f.setsampwidth(2);f.setframerate(16000);f.writeframes(samples.tobytes())
        # Recreate the reference from the immutable source every run; never filter a processed cache twice.
        filtered=target.with_name(target.stem+'-filtered.wav')
        subprocess.run(['ffmpeg','-v','error','-y','-i',str(target),'-af',manifest['processing']['filter'],'-ar','16000','-ac','1','-c:a','pcm_s16le',str(filtered)],check=True)
        # Preserve the audition's PCM16 boundary before its separate level trim.
        subprocess.run(['ffmpeg','-v','error','-y','-i',str(filtered),'-af',manifest['processing']['trim_filter'],'-ar','16000','-ac','1','-c:a','pcm_s16le',str(target)],check=True)
        with wave.open(str(target)) as f:
            assert (f.getframerate(),f.getnchannels(),f.getsampwidth())==(16000,1,2)
            processed=np.frombuffer(f.readframes(f.getnframes()),dtype='<i2')
        assert len(processed)==len(samples), 'Denoising must preserve clip timing'
        # Preserve the author-selected B baseline before the explicit fixed gain.
        if id==0:
            approved=json.loads((out/'approved-sample.json').read_text())
            baseline=encode(processed[:approved['samples']])
            assert hashlib.sha256(baseline).hexdigest()==approved['adpcm_sha256'], 'B baseline changed'
            manifest['processing']['base_audition_adpcm_sha256']=hashlib.sha256(baseline).hexdigest()
        baseline_sha=hashlib.sha256(processed.tobytes()).hexdigest()
        adapted=target.with_name(target.stem+'-speaker.wav')
        chain=manifest['processing']['speaker_filter']+f',aresample=16000,apad=whole_len={len(processed)},atrim=end_sample={len(processed)}'
        subprocess.run(['ffmpeg','-v','error','-y','-i',str(target),'-af',chain,'-ar','16000','-ac','1','-c:a','pcm_s16le',str(adapted)],check=True)
        with wave.open(str(adapted)) as f:samples=np.frombuffer(f.readframes(f.getnframes()),dtype='<i2')
        assert len(samples)==len(processed) and max(abs(samples.astype(np.int32)))<13000
        adapted.replace(target)
        data=encode(samples);asset=out/f'clip_{id:02}.adpcm';asset.write_bytes(data)
        symbol=f'_binary_clip_{id:02}_adpcm_start';declarations.append(f'extern const uint8_t {symbol}[];\n');table.append(f'    {{{symbol}, {len(data)}, {len(samples)}}},\n')
        manifest['clips'].append({'id':id,'park':('九寨沟','黄龙')[park],'name':name,'text':text,'samples':len(samples),'seconds':round(len(samples)/16000,3),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'baseline_pcm_sha256':baseline_sha})
        print(f'{id:02} {name}: {len(samples)/16000:.2f}s, {len(data)} bytes',flush=True)
    declarations+=['const guide_voice_clip_t guide_voice_clips[GUIDE_SPOTS] = {\n',*table,'};\n']
    (root/'main/guide_voice_assets.c').write_text(''.join(declarations));(out/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
    print('TOTAL',sum(c['bytes'] for c in manifest['clips']),'bytes',flush=True)

if __name__=='__main__':main()
