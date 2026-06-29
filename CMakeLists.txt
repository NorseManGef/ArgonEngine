cmake_minimum_required(VERSION 3.20)
project(argonengine)

find_package(OpenGL REQUIRED)
if(APPLE)
find_package(CURL REQUIRED)
pkg_check_modules(GLEW REQUIRED IMPORTED_TARGET glew)  
endif(APPLE)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})
file(MAKE_DIRECTORY ${GENERATED_DIR}/build_config)

add_subdirectory(common/bullet3)

set(CMAKE_BUILD_TYPE Debug)

if(WIN32)
set(INC_OS win)
set(INC_OS_ADD
    win/SDL3/hidapi/windows
)
endif(WIN32)

if(UNIX AND NOT APPLE)
set(INC_OS linux)
set(INC_OS_ADD
    # for SDL
    ${GENERATED_DIR}/build_config
)
endif(UNIX AND NOT APPLE)

if(APPLE)
set(INC_OS mac)
set(INC_OS_ADD
    # nothing
)
endif(APPLE)

set(INC_SDL
    common/SDL3
    common/SDL3/audio
    common/SDL3/build_config
    common/SDL3/camera
    common/SDL3/dynapi
    common/SDL3/dialog
    common/SDL3/filesystem
    common/SDL3/gpu
    common/SDL3/haptic
    common/SDL3/hidapi
    common/SDL3/hidapi/hidapi
    common/SDL3/io
    common/SDL3/joystick
    common/SDL3/locale
    common/SDL3/main
    common/SDL3/misc
    common/SDL3/power
    common/SDL3/process
    common/SDL3/render
    common/SDL3/sensor
    common/SDL3/thread
    common/SDL3/time
    common/SDL3/timer
    common/SDL3/tray
    common/SDL3/video
    common/SDL3/video/khronos
    ${INC_OS}/SDL3
    ${INC_OS}/SDL3/audio
    ${INC_OS}/SDL3/camera
    ${INC_OS}/SDL3/hidapi
    ${INC_OS}/SDL3/hidapi/${INC_OS}
    ${INC_OS}/SDL3/joystick
    ${INC_OS}/SDL3/thread
    ${INC_OS}/SDL3/time
    ${INC_OS}/SDL3/timer
    ${INC_OS}/SDL3/video
)

set(INC_STB
    common/stb
)

set(INC_LIBUSB
    common/libusb
    ${INC_OS}/libusb)

include_directories(
    common
    app
    ${OPENGL_INCLUDE_DIR}
    ${INC_OS}
    ${INC_OS_ADD}
    ${INC_SDL}
    ${INC_LIBUSB}
    ${INC_STB}
    ${GENERATED_DIR}
)


set(SRC_ARGONENGINE
    common/ArgonEngine/ArgonInit.cpp
    common/ArgonEngine/AudioSystem.cpp
    common/ArgonEngine/BasicShape.cpp
    common/ArgonEngine/BoundingCube.cpp
    common/ArgonEngine/Collision.cpp
    common/ArgonEngine/Curves.cpp
    common/ArgonEngine/Hardware.cpp
    common/ArgonEngine/MeshFormat.cpp
    common/ArgonEngine/Node.cpp
    common/ArgonEngine/OpenGLES.cpp
    common/ArgonEngine/Reflection.cpp
    common/ArgonEngine/RenderSystem.cpp
    common/ArgonEngine/RenderSystemConstants.cpp
    common/ArgonEngine/ScoreController.cpp
    common/ArgonEngine/Sprite.cpp
    common/ArgonEngine/Text.cpp
    common/ArgonEngine/Thread.cpp
    common/ArgonEngine/UserInterface.cpp
    common/ArgonEngine/UserMusic.cpp
    common/ArgonEngine/Utility.cpp
    common/ArgonEngine/VirtualResource.cpp
    common/ArgonEngine/VirtualResourceImage.cpp
)

set(SRC_ARGONAPP
    app/ArgonAppBase.cpp
    app/ArgonAppFlowControl.cpp
    app/main.cpp
)

set(SRC_JSON
    common/json/json_reader.cpp
    common/json/json_value.cpp
    common/json/json_writer.cpp
)

set(SRC_OGG
    common/ogg/bitwise.c
    common/ogg/framing.c
)

SET(SRC_OPUS
    common/opus/analysis.c
    common/opus/celt/arm/arm_celt_map.c
    common/opus/celt/arm/armcpu.c
    common/opus/celt/bands.c
    common/opus/celt/celt.c
    common/opus/celt/celt_decoder.c
    common/opus/celt/celt_encoder.c
    common/opus/celt/celt_lpc.c
    common/opus/celt/cwrs.c
    common/opus/celt/entcode.c
    common/opus/celt/entdec.c
    common/opus/celt/entenc.c
    common/opus/celt/kiss_fft.c
    common/opus/celt/laplace.c
    common/opus/celt/mathops.c
    common/opus/celt/mdct.c
    common/opus/celt/modes.c
    common/opus/celt/pitch.c
    common/opus/celt/quant_bands.c
    common/opus/celt/rate.c
    common/opus/celt/vq.c
    common/opus/mlp.c
    common/opus/mlp_data.c
    common/opus/opus.c
    common/opus/opus_decoder.c
    common/opus/opus_encoder.c
    common/opus/opus_multistream.c
    common/opus/opus_multistream_decoder.c
    common/opus/opus_multistream_encoder.c
    common/opus/repacketizer.c
    common/opus/silk/A2NLSF.c
    common/opus/silk/ana_filt_bank_1.c
    common/opus/silk/biquad_alt.c
    common/opus/silk/bwexpander.c
    common/opus/silk/bwexpander_32.c
    common/opus/silk/check_control_input.c
    common/opus/silk/CNG.c
    common/opus/silk/code_signs.c
    common/opus/silk/control_audio_bandwidth.c
    common/opus/silk/control_codec.c
    common/opus/silk/control_SNR.c
    common/opus/silk/debug.c
    common/opus/silk/dec_API.c
    common/opus/silk/decode_core.c
    common/opus/silk/decode_frame.c
    common/opus/silk/decode_indices.c
    common/opus/silk/decode_parameters.c
    common/opus/silk/decode_pitch.c
    common/opus/silk/decode_pulses.c
    common/opus/silk/decoder_set_fs.c
    common/opus/silk/enc_API.c
    common/opus/silk/encode_indices.c
    common/opus/silk/encode_pulses.c
    common/opus/silk/fixed/apply_sine_window_FIX.c
    common/opus/silk/fixed/autocorr_FIX.c
    common/opus/silk/fixed/burg_modified_FIX.c
    common/opus/silk/fixed/corrMatrix_FIX.c
    common/opus/silk/fixed/encode_frame_FIX.c
    common/opus/silk/fixed/find_LPC_FIX.c
    common/opus/silk/fixed/find_LTP_FIX.c
    common/opus/silk/fixed/find_pitch_lags_FIX.c
    common/opus/silk/fixed/find_pred_coefs_FIX.c
    common/opus/silk/fixed/k2a_FIX.c
    common/opus/silk/fixed/k2a_Q16_FIX.c
    common/opus/silk/fixed/LTP_analysis_filter_FIX.c
    common/opus/silk/fixed/LTP_scale_ctrl_FIX.c
    common/opus/silk/fixed/noise_shape_analysis_FIX.c
    common/opus/silk/fixed/pitch_analysis_core_FIX.c
    common/opus/silk/fixed/prefilter_FIX.c
    common/opus/silk/fixed/process_gains_FIX.c
    common/opus/silk/fixed/regularize_correlations_FIX.c
    common/opus/silk/fixed/residual_energy16_FIX.c
    common/opus/silk/fixed/residual_energy_FIX.c
    common/opus/silk/fixed/schur64_FIX.c
    common/opus/silk/fixed/schur_FIX.c
    common/opus/silk/fixed/solve_LS_FIX.c
    common/opus/silk/fixed/vector_ops_FIX.c
    common/opus/silk/fixed/warped_autocorrelation_FIX.c
    common/opus/silk/float/apply_sine_window_FLP.c
    common/opus/silk/float/autocorrelation_FLP.c
    common/opus/silk/float/burg_modified_FLP.c
    common/opus/silk/float/bwexpander_FLP.c
    common/opus/silk/float/corrMatrix_FLP.c
    common/opus/silk/float/encode_frame_FLP.c
    common/opus/silk/float/energy_FLP.c
    common/opus/silk/float/find_LPC_FLP.c
    common/opus/silk/float/find_LTP_FLP.c
    common/opus/silk/float/find_pitch_lags_FLP.c
    common/opus/silk/float/find_pred_coefs_FLP.c
    common/opus/silk/float/inner_product_FLP.c
    common/opus/silk/float/k2a_FLP.c
    common/opus/silk/float/levinsondurbin_FLP.c
    common/opus/silk/float/LPC_analysis_filter_FLP.c
    common/opus/silk/float/LPC_inv_pred_gain_FLP.c
    common/opus/silk/float/LTP_analysis_filter_FLP.c
    common/opus/silk/float/LTP_scale_ctrl_FLP.c
    common/opus/silk/float/noise_shape_analysis_FLP.c
    common/opus/silk/float/pitch_analysis_core_FLP.c
    common/opus/silk/float/prefilter_FLP.c
    common/opus/silk/float/process_gains_FLP.c
    common/opus/silk/float/regularize_correlations_FLP.c
    common/opus/silk/float/residual_energy_FLP.c
    common/opus/silk/float/scale_copy_vector_FLP.c
    common/opus/silk/float/scale_vector_FLP.c
    common/opus/silk/float/schur_FLP.c
    common/opus/silk/float/solve_LS_FLP.c
    common/opus/silk/float/sort_FLP.c
    common/opus/silk/float/warped_autocorrelation_FLP.c
    common/opus/silk/float/wrappers_FLP.c
    common/opus/silk/gain_quant.c
    common/opus/silk/HP_variable_cutoff.c
    common/opus/silk/init_decoder.c
    common/opus/silk/init_encoder.c
    common/opus/silk/inner_prod_aligned.c
    common/opus/silk/interpolate.c
    common/opus/silk/lin2log.c
    common/opus/silk/log2lin.c
    common/opus/silk/LP_variable_cutoff.c
    common/opus/silk/LPC_analysis_filter.c
    common/opus/silk/LPC_inv_pred_gain.c
    common/opus/silk/NLSF2A.c
    common/opus/silk/NLSF_decode.c
    common/opus/silk/NLSF_del_dec_quant.c
    common/opus/silk/NLSF_encode.c
    common/opus/silk/NLSF_stabilize.c
    common/opus/silk/NLSF_unpack.c
    common/opus/silk/NLSF_VQ.c
    common/opus/silk/NLSF_VQ_weights_laroia.c
    common/opus/silk/NSQ.c
    common/opus/silk/NSQ_del_dec.c
    common/opus/silk/pitch_est_tables.c
    common/opus/silk/PLC.c
    common/opus/silk/process_NLSFs.c
    common/opus/silk/quant_LTP_gains.c
    common/opus/silk/resampler.c
    common/opus/silk/resampler_down2.c
    common/opus/silk/resampler_down2_3.c
    common/opus/silk/resampler_private_AR2.c
    common/opus/silk/resampler_private_down_FIR.c
    common/opus/silk/resampler_private_IIR_FIR.c
    common/opus/silk/resampler_private_up2_HQ.c
    common/opus/silk/resampler_rom.c
    common/opus/silk/shell_coder.c
    common/opus/silk/sigm_Q15.c
    common/opus/silk/sort.c
    common/opus/silk/stereo_decode_pred.c
    common/opus/silk/stereo_encode_pred.c
    common/opus/silk/stereo_find_predictor.c
    common/opus/silk/stereo_LR_to_MS.c
    common/opus/silk/stereo_MS_to_LR.c
    common/opus/silk/stereo_quant_pred.c
    common/opus/silk/sum_sqr_shift.c
    common/opus/silk/table_LSF_cos.c
    common/opus/silk/tables_gain.c
    common/opus/silk/tables_LTP.c
    common/opus/silk/tables_NLSF_CB_NB_MB.c
    common/opus/silk/tables_NLSF_CB_WB.c
    common/opus/silk/tables_other.c
    common/opus/silk/tables_pitch_lag.c
    common/opus/silk/tables_pulses_per_block.c
    common/opus/silk/VAD.c
    common/opus/silk/VQ_WMat_EC.c 
)

set(SRC_OPUSFILE
    common/opusfile/http.c
    common/opusfile/info.c
    common/opusfile/internal.c
    common/opusfile/opusfile.c
    common/opusfile/stream.c
)

set(SRC_SDL3
    common/SDL3/SDL.c
    common/SDL3/SDL_assert.c
    common/SDL3/SDL_error.c
    common/SDL3/SDL_guid.c
    common/SDL3/SDL_hashtable.c
    common/SDL3/SDL_hints.c
    common/SDL3/SDL_list.c
    common/SDL3/SDL_log.c
    common/SDL3/SDL_properties.c
    common/SDL3/SDL_utils.c
    common/SDL3/atomic/SDL_atomic.c
    common/SDL3/atomic/SDL_spinlock.c
    common/SDL3/audio/SDL_audio.c
    common/SDL3/audio/SDL_audiocvt.c
    common/SDL3/audio/SDL_audiodev.c
    common/SDL3/audio/SDL_audioqueue.c
    common/SDL3/audio/SDL_audioresample.c
    common/SDL3/audio/SDL_audiotypecvt.c
    common/SDL3/audio/SDL_mixer.c
    common/SDL3/audio/SDL_wave.c
    common/SDL3/audio/disk/SDL_diskaudio.c
    common/SDL3/audio/dsp/SDL_dspaudio.c
    common/SDL3/audio/dummy/SDL_dummyaudio.c
    common/SDL3/audio/sndio/SDL_sndioaudio.c
    common/SDL3/camera/SDL_camera.c
    common/SDL3/camera/dummy/SDL_camera_dummy.c
    common/SDL3/core/SDL_core_unsupported.c
    common/SDL3/cpuinfo/SDL_cpuinfo.c
    common/SDL3/dialog/SDL_dialog.c
    common/SDL3/dialog/SDL_dialog_utils.c
    common/SDL3/dynapi/SDL_dynapi.c
    common/SDL3/events/SDL_categories.c
    common/SDL3/events/SDL_clipboardevents.c
    common/SDL3/events/SDL_displayevents.c
    common/SDL3/events/SDL_dropevents.c
    common/SDL3/events/SDL_events.c
    common/SDL3/events/SDL_eventwatch.c
    common/SDL3/events/SDL_keyboard.c
    common/SDL3/events/SDL_keymap.c
    common/SDL3/events/SDL_keysym_to_keycode.c
    common/SDL3/events/SDL_keysym_to_scancode.c
    common/SDL3/events/SDL_mouse.c
    common/SDL3/events/SDL_pen.c
    common/SDL3/events/SDL_quit.c
    common/SDL3/events/SDL_scancode_tables.c
    common/SDL3/events/SDL_touch.c
    common/SDL3/events/SDL_windowevents.c
    common/SDL3/events/imKStoUCS.c
    common/SDL3/filesystem/SDL_filesystem.c
    common/SDL3/gpu/SDL_gpu.c
    common/SDL3/gpu/vulkan/SDL_gpu_vulkan.c
    common/SDL3/haptic/SDL_haptic.c
    common/SDL3/haptic/hidapi/SDL_hidapihaptic.c
    common/SDL3/haptic/hidapi/SDL_hidapihaptic_lg4ff.c
    common/SDL3/hidapi/SDL_hidapi.c
    common/SDL3/io/SDL_asyncio.c
    common/SDL3/io/SDL_iostream.c
    common/SDL3/io/generic/SDL_asyncio_generic.c
    common/SDL3/joystick/SDL_gamepad.c
    common/SDL3/joystick/SDL_joystick.c
    common/SDL3/joystick/SDL_steam_virtual_gamepad.c
    common/SDL3/joystick/controller_type.c
    common/SDL3/joystick/dummy/SDL_sysjoystick.c
    common/SDL3/joystick/hidapi/SDL_hidapi_8bitdo.c
    common/SDL3/joystick/hidapi/SDL_hidapi_combined.c
    common/SDL3/joystick/hidapi/SDL_hidapi_flydigi.c
    common/SDL3/joystick/hidapi/SDL_hidapi_gamecube.c
    common/SDL3/joystick/hidapi/SDL_hidapi_gip.c
    common/SDL3/joystick/hidapi/SDL_hidapi_lg4ff.c
    common/SDL3/joystick/hidapi/SDL_hidapi_luna.c
    common/SDL3/joystick/hidapi/SDL_hidapi_ps3.c
    common/SDL3/joystick/hidapi/SDL_hidapi_ps4.c
    common/SDL3/joystick/hidapi/SDL_hidapi_ps5.c
    common/SDL3/joystick/hidapi/SDL_hidapi_rumble.c
    common/SDL3/joystick/hidapi/SDL_hidapi_shield.c
    common/SDL3/joystick/hidapi/SDL_hidapi_sinput.c
    common/SDL3/joystick/hidapi/SDL_hidapi_stadia.c
    common/SDL3/joystick/hidapi/SDL_hidapi_steam.c
    common/SDL3/joystick/hidapi/SDL_hidapi_steam_hori.c
    common/SDL3/joystick/hidapi/SDL_hidapi_steam_triton.c
    common/SDL3/joystick/hidapi/SDL_hidapi_steamdeck.c
    common/SDL3/joystick/hidapi/SDL_hidapi_switch.c
    common/SDL3/joystick/hidapi/SDL_hidapi_switch2.c
    common/SDL3/joystick/hidapi/SDL_hidapi_wii.c
    common/SDL3/joystick/hidapi/SDL_hidapi_xbox360.c
    common/SDL3/joystick/hidapi/SDL_hidapi_xbox360w.c
    common/SDL3/joystick/hidapi/SDL_hidapi_xboxone.c
    common/SDL3/joystick/hidapi/SDL_hidapi_zuiki.c
    common/SDL3/joystick/hidapi/SDL_hidapijoystick.c
    common/SDL3/joystick/hidapi/SDL_report_descriptor.c
    common/SDL3/joystick/virtual/SDL_virtualjoystick.c
    common/SDL3/libm/e_atan2.c
    common/SDL3/libm/e_exp.c
    common/SDL3/libm/e_fmod.c
    common/SDL3/libm/e_log.c
    common/SDL3/libm/e_log10.c
    common/SDL3/libm/e_pow.c
    common/SDL3/libm/e_rem_pio2.c
    common/SDL3/libm/e_sqrt.c
    common/SDL3/libm/k_cos.c
    common/SDL3/libm/k_rem_pio2.c
    common/SDL3/libm/k_sin.c
    common/SDL3/libm/k_tan.c
    common/SDL3/libm/s_atan.c
    common/SDL3/libm/s_copysign.c
    common/SDL3/libm/s_cos.c
    common/SDL3/libm/s_fabs.c
    common/SDL3/libm/s_floor.c
    common/SDL3/libm/s_isinf.c
    common/SDL3/libm/s_isinff.c
    common/SDL3/libm/s_isnan.c
    common/SDL3/libm/s_isnanf.c
    common/SDL3/libm/s_modf.c
    common/SDL3/libm/s_scalbn.c
    common/SDL3/libm/s_sin.c
    common/SDL3/libm/s_tan.c
    common/SDL3/locale/SDL_locale.c
    common/SDL3/main/SDL_main_callbacks.c
    common/SDL3/main/SDL_runapp.c
    common/SDL3/main/generic/SDL_sysmain_callbacks.c
    common/SDL3/misc/SDL_libusb.c
    common/SDL3/misc/SDL_url.c
    common/SDL3/power/SDL_power.c
    common/SDL3/process/SDL_process.c
    common/SDL3/render/SDL_render.c
    common/SDL3/render/SDL_render_unsupported.c
    common/SDL3/render/SDL_yuv_sw.c
    common/SDL3/render/gpu/SDL_pipeline_gpu.c
    common/SDL3/render/gpu/SDL_render_gpu.c
    common/SDL3/render/gpu/SDL_shaders_gpu.c
    common/SDL3/render/opengl/SDL_render_gl.c
    common/SDL3/render/opengl/SDL_shaders_gl.c
    common/SDL3/render/opengles2/SDL_render_gles2.c
    common/SDL3/render/opengles2/SDL_shaders_gles2.c
    common/SDL3/render/software/SDL_blendfillrect.c
    common/SDL3/render/software/SDL_blendline.c
    common/SDL3/render/software/SDL_blendpoint.c
    common/SDL3/render/software/SDL_drawline.c
    common/SDL3/render/software/SDL_drawpoint.c
    common/SDL3/render/software/SDL_render_sw.c
    common/SDL3/render/software/SDL_triangle.c
    common/SDL3/render/vulkan/SDL_render_vulkan.c
    common/SDL3/render/vulkan/SDL_shaders_vulkan.c
    common/SDL3/sensor/SDL_sensor.c
    common/SDL3/sensor/dummy/SDL_dummysensor.c
    common/SDL3/stdlib/SDL_crc16.c
    common/SDL3/stdlib/SDL_crc32.c
    common/SDL3/stdlib/SDL_getenv.c
    common/SDL3/stdlib/SDL_iconv.c
    common/SDL3/stdlib/SDL_malloc.c
    common/SDL3/stdlib/SDL_memcpy.c
    common/SDL3/stdlib/SDL_memmove.c
    common/SDL3/stdlib/SDL_memset.c
    common/SDL3/stdlib/SDL_mslibc.c
    common/SDL3/stdlib/SDL_murmur3.c
    common/SDL3/stdlib/SDL_qsort.c
    common/SDL3/stdlib/SDL_random.c
    common/SDL3/stdlib/SDL_stdlib.c
    common/SDL3/stdlib/SDL_string.c
    common/SDL3/stdlib/SDL_strtokr.c
    common/SDL3/storage/SDL_storage.c
    common/SDL3/storage/generic/SDL_genericstorage.c
    common/SDL3/storage/steam/SDL_steamstorage.c
    common/SDL3/test/SDL_test_assert.c
    common/SDL3/test/SDL_test_common.c
    common/SDL3/test/SDL_test_compare.c
    common/SDL3/test/SDL_test_crc32.c
    common/SDL3/test/SDL_test_font.c
    common/SDL3/test/SDL_test_fuzzer.c
    common/SDL3/test/SDL_test_harness.c
    common/SDL3/test/SDL_test_log.c
    common/SDL3/test/SDL_test_md5.c
    common/SDL3/test/SDL_test_memory.c
    common/SDL3/thread/SDL_thread.c
    common/SDL3/time/SDL_time.c
    common/SDL3/timer/SDL_timer.c
    common/SDL3/tray/SDL_tray_utils.c
    common/SDL3/video/SDL_RLEaccel.c
    common/SDL3/video/SDL_blit.c
    common/SDL3/video/SDL_blit_0.c
    common/SDL3/video/SDL_blit_1.c
    common/SDL3/video/SDL_blit_A.c
    common/SDL3/video/SDL_blit_N.c
    common/SDL3/video/SDL_blit_auto.c
    common/SDL3/video/SDL_blit_copy.c
    common/SDL3/video/SDL_blit_slow.c
    common/SDL3/video/SDL_bmp.c
    common/SDL3/video/SDL_clipboard.c
    common/SDL3/video/SDL_egl.c
    common/SDL3/video/SDL_fillrect.c
    common/SDL3/video/SDL_pixels.c
    common/SDL3/video/SDL_rect.c
    common/SDL3/video/SDL_rotate.c
    common/SDL3/video/SDL_stb.c
    common/SDL3/video/SDL_stretch.c
    common/SDL3/video/SDL_surface.c
    common/SDL3/video/SDL_video.c
    common/SDL3/video/SDL_video_unsupported.c
    common/SDL3/video/SDL_vulkan_utils.c
    common/SDL3/video/SDL_yuv.c
    common/SDL3/video/dummy/SDL_nullevents.c
    common/SDL3/video/dummy/SDL_nullframebuffer.c
    common/SDL3/video/dummy/SDL_nullvideo.c
    common/SDL3/video/offscreen/SDL_offscreenevents.c
    common/SDL3/video/offscreen/SDL_offscreenframebuffer.c
    common/SDL3/video/offscreen/SDL_offscreenopengles.c
    common/SDL3/video/offscreen/SDL_offscreenvideo.c
    common/SDL3/video/offscreen/SDL_offscreenvulkan.c
    common/SDL3/video/offscreen/SDL_offscreenwindow.c
    common/SDL3/video/openvr/SDL_openvrvideo.c
    common/SDL3/video/yuv2rgb/yuv_rgb_lsx.c
    common/SDL3/video/yuv2rgb/yuv_rgb_sse.c
    common/SDL3/video/yuv2rgb/yuv_rgb_std.c
)

if(UNIX AND NOT APPLE)
  include(linux/cmake/defines.cmake)
  configure_file("common/SDL3/build_config/SDL_build_config.h.cmake"
    "${GENERATED_DIR}/SDL_build_config.h.intermediate")
  # 2. generate SDL_build_config.h in an build_type-dependent folder (which should be first in the include search path)
  file(GENERATE
    OUTPUT "${GENERATED_DIR}/build_config/SDL_build_config.h"
    INPUT "${GENERATED_DIR}/SDL_build_config.h.intermediate"
  )
endif(UNIX AND NOT APPLE)

set(SRC_LIBUSB
    common/libusb/core.c
    common/libusb/hotplug.c
    common/libusb/descriptor.c
    common/libusb/io.c
    common/libusb/strerror.c
    common/libusb/sync.c
)

set(SRC_STB
    common/stb/stb_image.c
    common/stb/stb_image_write.c
    common/stb/stb_truetype.c
)

set(SRC_VORBIS
    common/vorbis/analysis.c
    common/vorbis/bitrate.c
    common/vorbis/block.c
    common/vorbis/codebook.c
    common/vorbis/envelope.c
    common/vorbis/floor0.c
    common/vorbis/floor1.c
    common/vorbis/info.c
    common/vorbis/lookup.c
    common/vorbis/lpc.c
    common/vorbis/lsp.c
    common/vorbis/mapping0.c
    common/vorbis/mdct.c
    common/vorbis/psy.c
    common/vorbis/registry.c
    common/vorbis/res0.c
    common/vorbis/sharedbook.c
    common/vorbis/smallft.c
    common/vorbis/synthesis.c
    common/vorbis/vorbisenc.c
    common/vorbis/vorbisfile.c
    common/vorbis/window.c
)

set(SRC_COMMON
    ${SRC_ARGONAPP}
    ${SRC_ARGONENGINE}
    ${SRC_BULLET}
    ${SRC_OGG}
    ${SRC_OPUS}
    ${SRC_OPUSFILE}
    ${SRC_JSON}
    ${SRC_STB}
    ${SRC_SDL3}
    ${SRC_LIBUSB}
    ${SRC_VORBIS}
)

set(SRC_WINDOWS
    win/GL/glew.c
    win/SDL3/audio/wasapi/SDL_wasapi.c
    win/SDL3/camera/mediafoundation/SDL_camera_mediafoundation.c
    win/SDL3/core/windows/SDL_hid.c
    win/SDL3/core/windows/SDL_immdevice.c
    win/SDL3/core/windows/SDL_windows.c
    win/SDL3/core/windows/SDL_xinput.c
    win/SDL3/core/windows/pch.c
    win/SDL3/dialog/windows/SDL_windowsdialog.c
    win/SDL3/filesystem/windows/SDL_sysfilesystem.c
    win/SDL3/filesystem/windows/SDL_sysfsops.c
    win/SDL3/gpu/d3d12/SDL_gpu_d3d12.c
    win/SDL3/haptic/windows/SDL_dinputhaptic.c
    win/SDL3/haptic/windows/SDL_windowshaptic.c
    win/SDL3/hidapi/windows/hidapi_descriptor_reconstruct.c
    win/SDL3/io/windows/SDL_asyncio_windows_ioring.c
    win/SDL3/joystick/windows/SDL_dinputjoystick.c
    win/SDL3/joystick/windows/SDL_rawinputjoystick.c
    win/SDL3/joystick/windows/SDL_windows_gaming_input.c
    win/SDL3/joystick/windows/SDL_windowsjoystick.c
    win/SDL3/joystick/windows/SDL_xinputjoystick.c
    win/SDL3/joystick/gdk/SDL_gameinputjoystick.cpp
    win/SDL3/core/windows/SDL_gameinput.cpp
    win/SDL3/loadso/windows/SDL_sysloadso.c
    win/SDL3/locale/windows/SDL_syslocale.c
    win/SDL3/main/windows/SDL_sysmain_runapp.c
    win/SDL3/misc/windows/SDL_sysurl.c
    win/SDL3/power/windows/SDL_syspower.c
    win/SDL3/process/windows/SDL_windowsprocess.c
    win/SDL3/render/direct3d/SDL_render_d3d.c
    win/SDL3/render/direct3d/SDL_shaders_d3d.c
    win/SDL3/render/direct3d11/SDL_render_d3d11.c
    win/SDL3/render/direct3d11/SDL_shaders_d3d11.c
    win/SDL3/render/direct3d12/SDL_render_d3d12.c
    win/SDL3/render/direct3d12/SDL_shaders_d3d12.c
    win/SDL3/sensor/windows/SDL_windowssensor.c
    win/SDL3/thread/windows/SDL_syscond_cv.c
    common/SDL3/thread/generic/SDL_sysrwlock.c
    common/SDL3/thread/generic/SDL_syscond.c
    win/SDL3/thread/windows/SDL_sysmutex.c
    win/SDL3/thread/windows/SDL_sysrwlock_srw.c
    win/SDL3/thread/windows/SDL_syssem.c
    win/SDL3/thread/windows/SDL_systhread.c
    win/SDL3/thread/windows/SDL_systls.c
    win/SDL3/time/windows/SDL_systime.c
    win/SDL3/timer/windows/SDL_systimer.c
    win/SDL3/tray/windows/SDL_tray.c
    win/SDL3/video/windows/SDL_windowsclipboard.c
    win/SDL3/video/windows/SDL_windowsevents.c
    win/SDL3/video/windows/SDL_windowsframebuffer.c
    win/SDL3/video/windows/SDL_windowskeyboard.c
    win/SDL3/video/windows/SDL_windowsmessagebox.c
    win/SDL3/video/windows/SDL_windowsmodes.c
    win/SDL3/video/windows/SDL_windowsmouse.c
    win/SDL3/video/windows/SDL_windowsopengl.c
    win/SDL3/video/windows/SDL_windowsopengles.c
    win/SDL3/video/windows/SDL_windowsrawinput.c
    win/SDL3/video/windows/SDL_windowsshape.c
    win/SDL3/video/windows/SDL_windowsvideo.c
    win/SDL3/video/windows/SDL_windowsvulkan.c
    win/SDL3/video/windows/SDL_windowswindow.c
    win/SDL3/video/windows/SDL_windowsgameinput.cpp
    win/libusb/os/events_windows.c
    win/libusb/os/threads_windows.c
    win/libusb/windows_common.c
    win/libusb/windows_usbdk.c
    win/libusb/windows_winusb.c
    win/libusb/windows_hotplug.c
)

set(SRC_LINUX
    linux/GL/glew.c
    linux/SDL3/audio/alsa/SDL_alsa_audio.c
    linux/SDL3/audio/jack/SDL_jackaudio.c
    linux/SDL3/audio/pipewire/SDL_pipewire.c
    linux/SDL3/audio/pulseaudio/SDL_pulseaudio.c
    linux/SDL3/camera/pipewire/SDL_camera_pipewire.c
    linux/SDL3/camera/v4l2/SDL_camera_v4l2.c
    linux/SDL3/core/linux/SDL_dbus.c
    linux/SDL3/core/linux/SDL_evdev.c
    linux/SDL3/core/linux/SDL_evdev_capabilities.c
    linux/SDL3/core/linux/SDL_evdev_kbd.c
    linux/SDL3/core/linux/SDL_fcitx.c
    linux/SDL3/core/linux/SDL_ibus.c
    linux/SDL3/core/linux/SDL_ime.c
    linux/SDL3/core/linux/SDL_progressbar.c
    linux/SDL3/core/linux/SDL_system_theme.c
    linux/SDL3/core/linux/SDL_threadprio.c
    linux/SDL3/core/linux/SDL_udev.c
    linux/SDL3/core/unix/SDL_appid.c
    linux/SDL3/core/unix/SDL_fribidi.c
    linux/SDL3/core/unix/SDL_gtk.c
    linux/SDL3/core/unix/SDL_libthai.c
    linux/SDL3/core/unix/SDL_poll.c
    linux/SDL3/dialog/unix/SDL_portaldialog.c
    linux/SDL3/dialog/unix/SDL_unixdialog.c
    linux/SDL3/dialog/unix/SDL_zenitydialog.c
    linux/SDL3/dialog/unix/SDL_zenitymessagebox.c
    linux/SDL3/filesystem/posix/SDL_sysfsops.c
    linux/SDL3/filesystem/unix/SDL_sysfilesystem.c
    linux/SDL3/haptic/linux/SDL_syshaptic.c
    linux/SDL3/hidapi/linux/hid.c
    linux/SDL3/io/io_uring/SDL_asyncio_liburing.c
    linux/SDL3/joystick/linux/SDL_sysjoystick.c
    linux/SDL3/loadso/dlopen/SDL_sysloadso.c
    linux/SDL3/locale/unix/SDL_syslocale.c
    linux/SDL3/misc/unix/SDL_sysurl.c
    linux/SDL3/power/linux/SDL_syspower.c
    linux/SDL3/process/posix/SDL_posixprocess.c
    linux/SDL3/thread/pthread/SDL_syscond.c
    linux/SDL3/thread/pthread/SDL_sysmutex.c
    linux/SDL3/thread/pthread/SDL_sysrwlock.c
    linux/SDL3/thread/pthread/SDL_syssem.c
    linux/SDL3/thread/pthread/SDL_systhread.c
    linux/SDL3/thread/pthread/SDL_systls.c
    linux/SDL3/time/unix/SDL_systime.c
    linux/SDL3/timer/unix/SDL_systimer.c
    linux/SDL3/tray/unix/SDL_tray.c
    linux/SDL3/video/wayland/SDL_waylandclipboard.c
    linux/SDL3/video/wayland/SDL_waylandcolor.c
    linux/SDL3/video/wayland/SDL_waylanddatamanager.c
    linux/SDL3/video/wayland/SDL_waylanddyn.c
    linux/SDL3/video/wayland/SDL_waylandevents.c
    linux/SDL3/video/wayland/SDL_waylandkeyboard.c
    linux/SDL3/video/wayland/SDL_waylandmessagebox.c
    linux/SDL3/video/wayland/SDL_waylandmouse.c
    linux/SDL3/video/wayland/SDL_waylandopengles.c
    linux/SDL3/video/wayland/SDL_waylandshmbuffer.c
    linux/SDL3/video/wayland/SDL_waylandvideo.c
    linux/SDL3/video/wayland/SDL_waylandvulkan.c
    linux/SDL3/video/wayland/SDL_waylandwindow.c
    linux/SDL3/video/x11/SDL_x11clipboard.c
    linux/SDL3/video/x11/SDL_x11dyn.c
    linux/SDL3/video/x11/SDL_x11events.c
    linux/SDL3/video/x11/SDL_x11framebuffer.c
    linux/SDL3/video/x11/SDL_x11keyboard.c
    linux/SDL3/video/x11/SDL_x11messagebox.c
    linux/SDL3/video/x11/SDL_x11modes.c
    linux/SDL3/video/x11/SDL_x11mouse.c
    linux/SDL3/video/x11/SDL_x11opengl.c
    linux/SDL3/video/x11/SDL_x11opengles.c
    linux/SDL3/video/x11/SDL_x11pen.c
    linux/SDL3/video/x11/SDL_x11settings.c
    linux/SDL3/video/x11/SDL_x11shape.c
    linux/SDL3/video/x11/SDL_x11toolkit.c
    linux/SDL3/video/x11/SDL_x11touch.c
    linux/SDL3/video/x11/SDL_x11video.c
    linux/SDL3/video/x11/SDL_x11vulkan.c
    linux/SDL3/video/x11/SDL_x11window.c
    linux/SDL3/video/x11/SDL_x11xfixes.c
    linux/SDL3/video/x11/SDL_x11xinput2.c
    linux/SDL3/video/x11/SDL_x11xsync.c
    linux/SDL3/video/x11/SDL_x11xtest.c
    linux/SDL3/video/x11/edid-parse.c
    linux/SDL3/video/x11/xsettings-client.c
    linux/libusb/os/events_posix.c
    linux/libusb/linux_netlink.c
    linux/libusb/linux_udev.c
    linux/libusb/linux_usbfs.c
    linux/libusb/os/threads_posix.c
)

set(SRC_MAC
    mac/SDL3/audio/coreaudio/SDL_coreaudio.m
    mac/SDL3/camera/coremedia/SDL_camera_coremedia.m
    mac/SDL3/core/unix/SDL_appid.c
    mac/SDL3/core/unix/SDL_poll.c
    mac/SDL3/dialog/cocoa/SDL_cocoadialog.m
    mac/SDL3/filesystem/cocoa/SDL_sysfilesystem.m
    mac/SDL3/filesystem/posix/SDL_sysfsops.c
    mac/SDL3/filesystem/unix/SDL_sysfilesystem.c
    mac/SDL3/gpu/metal/SDL_gpu_metal.m
    mac/SDL3/haptic/darwin/SDL_syshaptic.c
    mac/SDL3/joystick/darwin/SDL_iokitjoystick.c
    mac/SDL3/joystick/apple/SDL_mfijoystick.m
    mac/SDL3/loadso/dlopen/SDL_sysloadso.c
    mac/SDL3/locale/macos/SDL_syslocale.m
    mac/SDL3/misc/macos/SDL_sysurl.m
    mac/SDL3/power/macos/SDL_syspower.c
    mac/SDL3/process/posix/SDL_posixprocess.c
    mac/SDL3/render/metal/SDL_render_metal.m
    mac/SDL3/sensor/coremotion/SDL_coremotionsensor.m
    mac/SDL3/thread/pthread/SDL_syscond.c
    mac/SDL3/thread/pthread/SDL_sysmutex.c
    mac/SDL3/thread/pthread/SDL_sysrwlock.c
    mac/SDL3/thread/pthread/SDL_syssem.c
    mac/SDL3/thread/pthread/SDL_systhread.c
    mac/SDL3/thread/pthread/SDL_systls.c
    mac/SDL3/time/unix/SDL_systime.c
    mac/SDL3/timer/unix/SDL_systimer.c
    mac/SDL3/tray/cocoa/SDL_tray.m
    mac/SDL3/video/cocoa/SDL_cocoaclipboard.m
    mac/SDL3/video/cocoa/SDL_cocoaevents.m
    mac/SDL3/video/cocoa/SDL_cocoakeyboard.m
    mac/SDL3/video/cocoa/SDL_cocoamessagebox.m
    mac/SDL3/video/cocoa/SDL_cocoametalview.m
    mac/SDL3/video/cocoa/SDL_cocoamodes.m
    mac/SDL3/video/cocoa/SDL_cocoamouse.m
    mac/SDL3/video/cocoa/SDL_cocoaopengl.m
    mac/SDL3/video/cocoa/SDL_cocoaopengles.m
    mac/SDL3/video/cocoa/SDL_cocoapen.m
    mac/SDL3/video/cocoa/SDL_cocoashape.m
    mac/SDL3/video/cocoa/SDL_cocoavideo.m
    mac/SDL3/video/cocoa/SDL_cocoavulkan.m
    mac/SDL3/video/cocoa/SDL_cocoawindow.m
    mac/libusb/os/events_posix.c
    mac/libusb/os/threads_posix.c
    mac/libusb/darwin_usb.c
)

if(WIN32)
set(SRC_OS ${SRC_WINDOWS})
endif(WIN32)

if(UNIX AND NOT APPLE)
set(SRC_OS ${SRC_LINUX})
endif(UNIX AND NOT APPLE)

if(APPLE)
set(SRC_OS ${SRC_MAC})
endif(APPLE)

set(LIB_WINDOWS
    Imm32
    windowscoreheadless
    WinMM
)

list(APPEND LIB_LINUX
    m
    dl
    rt
)

set(LIB_MAC
    PkgConfig::GLEW
    CURL::libcurl
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreFoundation"
    "-framework CoreAudio"
    "-framework CoreVideo"
    "-framework CoreMedia"
    "-framework Metal"
    "-framework QuartzCore"
    "-framework ForceFeedback"
    "-framework GameController"
    "-framework AVFoundation"
    "-framework AudioToolbox"
    "-framework Security"
    "-framework Carbon"
    "-framework UniformTypeIdentifiers"
    "-framework CoreHaptics"
    "-framework OpenGL"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
set(LIB_OS ${LIB_WINDOWS})
endif(WIN32)

if(UNIX AND NOT APPLE)
set(LIB_OS ${LIB_LINUX})
endif(UNIX AND NOT APPLE)

if(APPLE)
set(LIB_OS ${LIB_MAC})
endif(APPLE)

if(UNIX AND NOT APPLE)
set(LIB_VULKAN libvulkan.so)
endif(UNIX AND NOT APPLE)

set(SOURCES
    ${SRC_COMMON}
    ${SRC_OS}
)

add_executable(argonapp ${SOURCES} ) 
target_link_libraries(argonapp 
    PRIVATE 
    BulletInverseDynamics
    BulletDynamics
    Bullet3Common
    BulletCollision
    BulletSoftBody
    LinearMath
)
add_compile_definitions(
    BT_ENABLE_PROFILE=1
)

include(common/bullet3/UseBullet.cmake)

if(NOT APPLE)
target_precompile_headers(argonapp
    PRIVATE
        common/precompiled.h
)
endif(NOT APPLE)

target_link_libraries(argonapp
    PRIVATE
        ${OPENGL_LIBRARIES}
        ${LIB_OS}
        ${LIB_VULKAN}
)

target_compile_definitions(argonapp
    PRIVATE
        "_UNICODE"
        "UNICODE"
)

if(WIN32)
add_compile_definitions(argonapp
    PRIVATE
        HAVE_STRUCT_TIMESPEC=1
        _TIMESPEC_DEFINED=1
        LIBUSB_WINDOWS_HOTPLUG
        DEFAULT_VISIBILITY=
        PLATFORM_WINDOWS=1
        SDL_PLATFORM_WIN32
        SDL_STATIC_LIB
        SDL_MAIN_USE_CALLBACKS
        SDL_MAIN_NOIMPL
        ARGON_WINDOW_HIGH_PIXEL_DENSITY
    )
endif(WIN32)

if(APPLE)
add_compile_definitions(argonapp
    PRIVATE
        DEFAULT_VISIBILITY=
)
endif(APPLE)

if(UNIX AND NOT APPLE)
add_compile_definitions(argonapp
    PRIVATE
        DEFAULT_VISIBILITY=
        _GNU_SOURCE
        HAVE_CLOCK_GETTIME
        ARGON_WINDOW_HIGH_PIXEL_DENSITY
)

if(WAYLAND_FOUND)
add_compile_definitions(argonapp
    PRIVATE
        "SDL_VIDEO_DRIVER_WAYLAND"
)

elseif(X11_FOUND)
add_compile_definitions(argonapp
    PRIVATE
        "SDL_VIDEO_DRIVER_X11"
)
else()
    message(FATAL_ERROR "No display backend available")
endif()
set(HAVE_LIBC 1)
endif(UNIX AND NOT APPLE)

if(UNIX AND NOT APPLE)
set_source_files_properties(${SRC_OPUS} PROPERTIES
    COMPILE_OPTIONS "-Wno-error=incompatible-pointer-types"
)
set_source_files_properties(${SRC_SDL3} PROPERTIES
    COMPILE_OPTIONS "-Wno-error=implicit-function-declaration"
                    "-Wno-error=incompatible-pointer-types"
                    "-Wno-error=int-conversion;-w"
)
set_source_files_properties(${SRC_OS} PROPERTIES
    COMPILE_OPTIONS "-Wno-error=incompatible-pointer-types;-Wno-error=int-conversion"
)
set_source_files_properties(${SRC_LIBUSB} PROPERTIES
    COMPILE_OPTIONS "-Wno-error=int-conversion"
)
endif(UNIX AND NOT APPLE)

if(APPLE)
set_source_files_properties(${SRC_BULLET} PROPERTIES
    COMPILE_OPTIONS "-Wno-error=c++11-narrowing"
)
set_source_files_properties(${SRC_MAC} PROPERTIES
    COMPILE_FLAGS "-fobjc-arc"
)
add_compile_definitions(argonapp
    PRIVATE
        "SDL_PLATFORM_MACOS"
        "SDL_VIDEO_OPENGL"
        "OPENGL_AUTO_VERSIONING"
)
endif(APPLE)

add_custom_command(TARGET argonapp POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${PROJECT_SOURCE_DIR}/resources" "$<TARGET_FILE_DIR:argonapp>/resources"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${PROJECT_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:argonapp>/shaders"
)
