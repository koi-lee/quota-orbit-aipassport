#!/usr/bin/env python3
"""Validate each embedded narration and compare the C decoder with FFmpeg IMA."""
import argparse
import hashlib
import json
import math
import struct
import subprocess
import tempfile
import wave
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--ffmpeg-compare',action='store_true');p.add_argument('--summary',action='store_true');p.add_argument('--decoder',required=True,type=Path);p.add_argument('--reference-dir',type=Path);p.add_argument('--baseline-dir',type=Path);p.add_argument('--wav-dir',type=Path);a=p.parse_args()
root=Path(__file__).resolve().parents[1];assets=root/'assets/voice';m=json.loads((assets/'manifest.json').read_text());assert len(m['clips'])==22
narration=json.loads((assets/'narration.json').read_text())
assert m['voice']==narration['voice']=='zf_001' and len(narration['clips'])==22
assert m['processing']['profile']=='B-light-denoise-v1' and m['processing']['dynamic_loudness'] is True
assert m['processing']['stage_pcm16_between_filters'] is True
assert m['processing']['linear_gain_db']==0.0
assert m['processing']['speaker_profile']=='peak-aware-speech-v1'
assert hashlib.sha256((assets/'narration.json').read_bytes()).hexdigest()==m['narration_sha256']
assert sum(c['samples']/16000 for c in m['clips'])<=narration['total_seconds_budget']==780
if a.wav_dir:a.wav_dir.mkdir(parents=True,exist_ok=True)
approved=json.loads((assets/'approved-sample.json').read_text())
assert approved['profile']==m['processing']['profile']
summary=[]
with tempfile.TemporaryDirectory(prefix='guide-voice-check-') as directory:
    tmp=Path(directory)
    for id,c in enumerate(m['clips']):
        source=assets/f'clip_{id:02}.adpcm';data=source.read_bytes()
        assert c['id']==id and len(data)==c['bytes']==(c['samples']+1)//2 and hashlib.sha256(data).hexdigest()==c['sha256']
        script=narration['clips'][id]
        assert c['text']==script['text'] and c['name']==script['name'] and c['park']==script['park']
        minimum,maximum=(45,60) if script['priority'] else (25,40)
        assert minimum<=c['samples']/16000<=maximum,(id,c['seconds'],minimum,maximum)
        raw=tmp/'c.raw';subprocess.run([str(a.decoder),str(source),str(c['samples']),str(raw)],check=True)
        pcm=raw.read_bytes();assert len(pcm)==c['samples']*2
        if id==approved['clip_id']:
            assert hashlib.sha256(c['text'].encode()).hexdigest()==approved['text_sha256']
            baseline=assets/'approved-sample.adpcm'
            assert hashlib.sha256(baseline.read_bytes()).hexdigest()==approved['adpcm_sha256']
            baseline_raw=tmp/'approved.raw';subprocess.run([str(a.decoder),str(baseline),str(approved['samples']),str(baseline_raw)],check=True)
            reference=baseline_raw.read_bytes();assert hashlib.sha256(reference).hexdigest()==approved['pcm_sha256'], 'Selected B baseline changed'
            assert m['processing']['base_audition_adpcm_sha256']==approved['adpcm_sha256']
        samples=struct.unpack('<%dh'%c['samples'],pcm);peak=max(map(abs,samples));assert 1000<peak<14000 and samples[0]==0 and abs(samples[-1])<100
        if a.ffmpeg_compare:
            # One standard WAV/IMA block, independent decoder, includes speech after leading silence.
            encoded=data[:8188];block=bytes(4)+encoded;block_samples=len(encoded)*2+1
            fmt=struct.pack('<HHIIHHHH',17,1,16000,8000,len(block),4,2,block_samples)
            def chunk(name,body):return name+struct.pack('<I',len(body))+body+(b'\0' if len(body)%2 else b'')
            body=b'WAVE'+chunk(b'fmt ',fmt)+chunk(b'fact',struct.pack('<I',block_samples))+chunk(b'data',block)
            (tmp/'reference.wav').write_bytes(b'RIFF'+struct.pack('<I',len(body))+body)
            subprocess.run(['ffmpeg','-v','error','-y','-i',str(tmp/'reference.wav'),'-f','s16le',str(tmp/'ffmpeg.raw')],check=True)
            expected=(tmp/'ffmpeg.raw').read_bytes()[2:];assert pcm[:len(expected)]==expected,(id,'IMA mismatch')
        snr=None
        if a.reference_dir:
            with wave.open(str(a.reference_dir/f'clip_{id:02}.wav')) as f:ref=struct.unpack('<%dh'%f.getnframes(),f.readframes(f.getnframes()))
            assert len(ref)==len(samples)
            if a.baseline_dir:
                with wave.open(str(a.baseline_dir/f'clip_{id:02}.wav')) as f:base=struct.unpack('<%dh'%f.getnframes(),f.readframes(f.getnframes()))
                assert len(base)==len(ref)
                assert hashlib.sha256(struct.pack('<%dh'%len(base),*base)).hexdigest()==c['baseline_pcm_sha256'], 'B baseline changed'
                # Speaker mastering may reshape dynamics, but must not drop voiced sections.
                active=covered=0;window=320
                base_energy=[sum(v*v for v in base[i:i+window])/len(base[i:i+window]) for i in range(0,len(base),window)]
                out_energy=[sum(v*v for v in ref[i:i+window])/len(ref[i:i+window]) for i in range(0,len(ref),window)]
                for j,e in enumerate(base_energy):
                    if e>500**2:
                        active+=1;covered+=max(out_energy[max(0,j-3):j+4])>150**2
                assert active and covered/active>.99, 'Voiced audio dropped during mastering'
            signal=sum(x*x for x in ref);noise=sum((x-y)**2 for x,y in zip(ref,samples));snr=10*math.log10(signal/max(noise,1));assert snr>20,(id,snr)
        if a.wav_dir:
            with wave.open(str(a.wav_dir/f'clip_{id:02}.wav'),'wb') as f:f.setnchannels(1);f.setsampwidth(2);f.setframerate(16000);f.writeframes(pcm)
        summary.append({'id':id,'name':c['name'],'seconds':c['seconds'],'peak':peak,'snr_db':round(snr,2) if snr else None,'ffmpeg_exact':a.ffmpeg_compare})
print(json.dumps({'clips':len(summary) if a.summary else summary,'bytes':sum(c['bytes'] for c in m['clips']),'result':'PASS'},ensure_ascii=False,indent=2))
