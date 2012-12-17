    desc = "The timing value $parameter";
    loggerOpts.add_options()
        ("$parameter",
        po::value<unsigned int>( )->default_value(0),
        desc.c_str() );

