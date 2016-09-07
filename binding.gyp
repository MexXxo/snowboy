{
  "targets": [
    {
      "target_name": "<(module_name)",
      "sources": [ "swig/Node/snowboy-detect-nan.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "<(module_root_dir)/",
        "<(module_root_dir)/swig/Node/portaudio/src/common/",
        "<(module_root_dir)/swig/Node/portaudio/include/"
      ],
      "libraries": [],
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
            "CoreServices.framework",
            "<(module_root_dir)/lib/osx/libsnowboy-detect.a",
            "<(module_root_dir)/swig/Node/portaudio/install/lib/libportaudio.a",
            ]
          }
        }]
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
