file(GLOB GUIDE_VOICE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/voice/clip_*.adpcm")
idf_component_register(
    SRCS "main.c" "guide_data.c" "guide_record.c" "guide_model.c" "guide_settings.c" "guide_ui.c" "guide_audio.c" "guide_adpcm.c" "guide_voice_assets.c" "../assets/guide/guide_art.c"
         "../assets/fonts/guide_font_12.c" "../assets/fonts/guide_font_14.c"
         "../assets/fonts/guide_font_16.c" "../assets/fonts/guide_font_20.c"
    EMBED_FILES ${GUIDE_VOICE_FILES}
    INCLUDE_DIRS "."
    REQUIRES bsp esp_timer nvs_flash
)
