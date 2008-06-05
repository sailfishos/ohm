audio_device_type(source).
audio_device_type(sink).

audio_device(sink  , private, ihfandheadset).
audio_device(sink  , private, bluetooth).
audio_device(sink  , private, headset).
audio_device(sink,   private, headphone).
audio_device(sink  , private, earpiece).
audio_device(sink  , public , ihf).
audio_device(source, private, bluetooth).
audio_device(source, private, headset).
audio_device(source, private, headmike).
audio_device(source, private, microphone).

accessory(ihfandheadset).
accessory(headset).
accessory(bluetooth).
accessory(headphone).
accessory(headmike).
