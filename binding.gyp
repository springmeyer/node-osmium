{
  'includes': [ 'common.gypi' ],
  'targets': [
    {
      'target_name': 'osmium',
      'sources': [
        "src/node_osmium.cpp"
      ],
      'include_dirs': [
          '../include/',
          '../libosmium/include/',
          './src/'
      ],
      'defines': [
        '_LARGEFILE_SOURCE',
        '_FILE_OFFSET_BITS=64',
        'OSMIUM_WITH_DEBUG'
      ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_RTTI': 'YES',
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'OTHER_CPLUSPLUSFLAGS':['-stdlib=libc++'],
        'OTHER_LDFLAGS':['-stdlib=libc++'],
        'CLANG_CXX_LANGUAGE_STANDARD':'c++11',
        'MACOSX_DEPLOYMENT_TARGET':'10.7'
      },
      'cflags_cc!': ['-fno-rtti', '-fno-exceptions'],
      'cflags_cc' : ['-std=c++11'],
      'libraries': [
          '-losmpbf',
          '-lprotobuf-lite',
          '-lexpat'
      ]
    },
    {
      'target_name': 'action_after_build',
      'type': 'none',
      'dependencies': [ 'osmium' ],
      'copies': [
        {
          'files': [ '<(PRODUCT_DIR)/osmium.node' ],
          'destination': './lib/'
        }
      ]
    }
  ]
}
