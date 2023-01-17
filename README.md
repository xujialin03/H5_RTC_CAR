# H5_RTC_CAR
node.js+h5+webrtc的遥控小车

需要使用Nginx 配置https wws
因为webrtc要调用摄像头需要https

HTTPS本地签名可以用openssl生成
nginx配置可参考 nginx配置文件夹内的内容修改

修改nginx配置需要将 控制端，手机端，arduino 服务器地址全部修改过来