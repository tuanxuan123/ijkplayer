整体编译步骤: 
1. 预编译openssl，vorbis，使用compile-ffmpeg.sh编译ffmpeg库，生成libavcodec.a, libswresample.a, libavutil.a, libavformat.a
2. 把上述的库拷到libffmpeg-all下，打开PixFFmpeg工程，编译生成一个整体的libPixFFmpeg.a，这个库整合了ffmpeg，openssl，vorbis
3. 需要对libPixFFmpeg.a进行符号加工，使用machorename工具加上"pvideo_"前缀，避免与其他库符号冲突，安装machorename工具，执行命令: MachoRenamer libPixFFmpeg.a -p pvideo_
4. 得到改符号后的libPixFfmpeg.a，同样拷到libffmpeg-all目录下，如果ffmpeg加了新接口，需要更新PixFFmpeg-prefix.pch到PixVideo_Framework工程
5. 使用PixVideo_Framework工程编译得到PixVideo.framework


版本号查询: strings xxx/xxx/PixVideo.framework/PixVideo | grep Version