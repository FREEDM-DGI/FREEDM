    desc = "The timing value $parameter";
    loggerOpts.add_options()
        ("$parameter",
        po::value<unsigned int>( ),
        desc.c_str() );

