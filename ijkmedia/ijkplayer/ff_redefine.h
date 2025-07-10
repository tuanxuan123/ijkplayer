#ifndef FF__FFMPEG_REDEFINE_H
#define FF__FFMPEG_REDEFINE_H



#ifdef TARGET_OS_IPHONE_UNUSE

#define     AVFormatContext            PdrAVFormatContext
#define     AVStream                   PdrAVStream
#define     AVCodecContext             PdrAVCodecContext
#define     AVPacket                   PdrAVPacket
#define     AVOutputFormat             PdrAVOutputFormat




// ------------------- pdr ffmpeg struch-------------------------
#define     AVDictionary               PdrAVDictionary
#define     AVCodecID                  PdrAVCodecID
#define     AVCodec                    PdrAVCodec
#define     AVSampleFormat             PdrAVSampleFormat
#define     AVSubtitle                 PdrAVSubtitle
#define     AVFrame                    PdrAVFrame
#define     AVCodecParameters          PdrAVCodecParameters
#define     AVClass                    PdrAVClass
#define     AVRational                 PdrAVRational
#define     AVInputFormat              PdrAVInputFormat
#define     AVApplicationContext       PdrAVApplicationContext
#define     AVBufferRef                PdrAVBufferRef




// -------------------  pdr ffmpeg function ----------------------
#define     avcodec_get_class                pdr_avcodec_get_class
#define     avcodec_find_encoder             pdr_avcodec_find_encoder
#define     avcodec_find_decoder             pdr_avcodec_find_decoder
#define     av_dict_set                      pdr_av_dict_set
#define     av_dict_get                      pdr_av_dict_get
#define     check_stream_specifier           pdr_check_stream_specifier
#define     av_opt_find                      pdr_av_opt_find
#define     filter_codec_opts                pdr_filter_codec_opts
#define     av_realloc_array                 pdr_av_realloc_array
#define     av_stream_get_side_data          pdr_av_stream_get_side_data
#define     av_strtod                        pdr_av_strtod
#define     av_display_rotation_get          pdr_av_display_rotation_get
#define     av_free                          pdr_av_free
#define     av_get_packed_sample_fmt         pdr_av_get_packed_sample_fmt

#define     av_packet_unref                  pdr_av_packet_unref
#define     av_init_packet                   pdr_av_init_packet
#define     av_freep                         pdr_av_freep
#define     avcodec_receive_frame            pdr_avcodec_receive_frame
#define     av_rescale_q                     pdr_av_rescale_q
#define     avcodec_flush_buffers            pdr_avcodec_flush_buffers

#define     av_packet_move_ref               pdr_av_packet_move_ref
#define     avcodec_decode_subtitle2         pdr_avcodec_decode_subtitle2
#define     avcodec_send_packet              pdr_avcodec_send_packet
#define     avcodec_free_context             pdr_avcodec_free_context
#define     av_frame_unref                   pdr_av_frame_unref
#define     avsubtitle_free                  pdr_avsubtitle_free
#define     av_frame_alloc                   pdr_av_frame_alloc
#define     av_frame_free                    pdr_av_frame_free
#define     av_gettime                       pdr_av_gettime
#define     swr_free                         pdr_swr_free
#define     av_rdft_end                      pdr_av_rdft_end
#define     av_gettime_relative              pdr_av_gettime_relative
#define     av_usleep                        pdr_av_usleep
#define     av_get_picture_type_char         pdr_av_get_picture_type_char
#define     av_image_copy                    pdr_av_image_copy
#define     av_frame_move_ref                pdr_av_frame_move_ref
#define     av_q2d                           pdr_av_q2d
#define     av_guess_frame_rate              pdr_av_guess_frame_rate
#define     swr_alloc_set_opts               pdr_swr_alloc_set_opts
#define     av_get_sample_fmt_name           pdr_av_get_sample_fmt_name
#define     av_dict_copy                     pdr_av_dict_copy
#define     av_opt_set_double                pdr_av_opt_set_double
#define     av_opt_set_dict                  pdr_av_opt_set_dict
#define     av_dict_free                     pdr_av_dict_free
#define     swr_init                         pdr_swr_init
#define     swr_set_compensation             pdr_swr_set_compensation
#define     av_fast_malloc                   pdr_av_fast_malloc
#define     swr_convert                      pdr_swr_convert
#define     av_get_bytes_per_sample          pdr_av_get_bytes_per_sample
#define     avcodec_alloc_context3           pdr_avcodec_alloc_context3
#define     avcodec_find_decoder_by_name     pdr_avcodec_find_decoder_by_name
#define     av_codec_get_max_lowres          pdr_av_codec_get_max_lowres
#define     av_dict_set_int                  pdr_av_dict_set_int
#define     avcodec_open2                    pdr_avcodec_open2
#define     avcodec_get_name                 pdr_avcodec_get_name
#define     av_stristart                     pdr_av_stristart
#define     av_find_input_format             pdr_av_find_input_format
#define     avformat_alloc_context           pdr_avformat_alloc_context
#define     avformat_open_input              pdr_avformat_open_input
#define     avformat_seek_file               pdr_avformat_seek_file
#define     av_dump_format                   pdr_av_dump_format
#define     av_find_best_stream              pdr_av_find_best_stream
#define     av_read_pause                    pdr_av_read_pause
#define     av_read_play                     pdr_av_read_play
#define     av_packet_ref                    pdr_av_packet_ref
#define     av_read_frame                    pdr_av_read_fra
#define     avio_feof                        pdr_avio_feof
#define     av_strdup                        pdr_av_strdup
#define     av_clip                          pdr_av_clip
#define     av_usleep                        pdr_av_usleep
#define     av_log_get_level                 pdr_av_log_get_level
#define     av_log_format_line               pdr_av_log_format_li
#define     avcodec_register_all             pdr_avcodec_register_all
#define     avfilter_register_all            pdr_avfilter_register_all
#define     av_register_all                  pdr_av_register_all
#define     avformat_network_init            pdr_avformat_network_init
#define     lockmgr_register                 pdr_av_lockmgr_register
#define     av_log_set_callback              pdr_av_log_set_callback
#define     av_init_packet                   pdr_av_init_packet
#define     avformat_network_deinit          pdr_avformat_network_deinit
#define     av_log_set_level                 pdr_av_log_set_level
#define     av_version_info                  pdr_av_version_info
#define     av_opt_set_defaults              pdr_av_opt_set_defaults
#define     mallocz                          pdr_av_mallocz
#define     application_closep               pdr_av_application_closep
#define     av_application_open              pdr_av_application_open
#define     avutil_version                   pdr_avutil_version
#define     avcodec_version                  pdr_avcodec_version
#define     avformat_version                 pdr_avformat_version
#define     swresample_version               pdr_swresample_version
#define     av_rescale                       pdr_av_rescale
#define     get_rotation                     pdr_get_rotation
#define     av_get_profile_name              pdr_av_get_profile_name
#define     av_get_pix_fmt_name              pdr_av_get_pix_fmt_name
#define     av_image_fill_arrays             pdr_av_image_fill_arrays
#define     av_image_get_buffer_size         pdr_av_image_get_buffer_size
#define     av_buffer_alloc                  pdr_av_buffer_alloc
#define     av_buffer_unref                  pdr_av_buffer_unref
#define     av_frame_ref                     pdr_av_frame_ref
#define     av_copy_packet                   pdr_av_copy_packet
#define     avio_open_dyn_buf                pdr_avio_open_dyn_buf
#define     avio_close_dyn_buf               pdr_avio_close_dyn_buf
#define     av_packet_get_side_data          pdr_av_packet_get_side_data
#define     avcodec_parameters_free          pdr_avcodec_parameters_free
#define     avcodec_parameters_alloc         pdr_avcodec_parameters_alloc
#define     avformat_close_input             pdr_avformat_close_input
#define     avformat_new_stream              pdr_avformat_new_stre
#define     avcodec_parameters_from_context                pdr_avcodec_parameters_from_context
#define     av_get_bits_per_sample                         pdr_av_get_bits_per_sample
#define     avio_find_protocol_name                        pdr_avio_find_protocol_name
#define     avformat_match_stream_specifier                pdr_avformat_match_stream_specifier
#define     avformat_find_stream_info                      pdr_avformat_find_stream_info
#define     setup_find_stream_info_opts                    pdr_setup_find_stream_info_opts
#define     av_format_inject_global_side_data              pdr_av_format_inject_global_side_data
#define     avcodec_parameters_to_context                  pdr_avcodec_parameters_to_context
#define     av_get_default_channel_layout                  pdr_av_get_default_channel_layout
#define     av_samples_get_buffer_size                     pdr_av_samples_get_buffer_size
#define     av_guess_sample_aspect_ratio                   pdr_av_guess_sample_aspect_ratio
#define     av_get_channel_layout_nb_channels              pdr_av_get_channel_layout_nb_channels  


#endif

#endif
