{
  "targets": [
    {
      "target_name": "snowboy",
      "sources": [ "snowboy-detect-nan.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "<!(node -e \"require('streaming-worker-sdk')\")",
        "<(module_root_dir)/../../",
        "portaudio/src/common/",
        "portaudio/include/"
      ],
      "libraries": [
        "<(module_root_dir)/../../lib/osx/libsnowboy-detect.a",
        "<(module_root_dir)/portaudio/install/lib/libportaudio.a",
      ],
      'xcode_settings': {
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
        'OTHER_CFLAGS': ['-stdlib=libc++', '-std=c++11']
      },
      "conditions": [
        ['OS=="mac"', {
          "link_settings": {
          "libraries": [
            "AudioUnit.framework",
            "Accelerate.framework",
            "CoreAudio.framework",
            "AudioToolbox.framework",
            "CoreServices.framework"
            ]
          }
        }]
      ]
    }
  ]
}
