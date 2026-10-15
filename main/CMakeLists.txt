idf_component_register(
    SRCS "main.c" "orbit_model.c" "orbit_language.c" "orbit_access.c" "orbit_net.c" "orbit_audio.c" "orbit_quota.c" "orbit_ui.c"
         "../assets/fonts/orbit_font_16.c" "../assets/fonts/orbit_font_14.c"
         "../assets/fonts/orbit_font_20.c" "../assets/fonts/orbit_font_12.c" "../assets/fonts/orbit_font_24.c" "../assets/character/orbit_archivist.c" "../assets/character/orbit_celebration.c" "../assets/wechat/orbit_miniapp.c"
         "../assets/fonts/orbit_font_tc_12.c"
         "../assets/fonts/orbit_font_tc_14.c"
         "../assets/fonts/orbit_font_tc_16.c"
         "../assets/fonts/orbit_font_tc_20.c"
         "../assets/fonts/orbit_font_tc_24.c"
    INCLUDE_DIRS "."
    REQUIRES bsp esp_event esp_netif esp_timer esp_wifi nvs_flash
             esp_http_client esp_http_server mbedtls lwip
)
