    try
    {
        $parameter = vm["$parameter"].as<unsigned int>();
    }
    catch (boost::bad_any_cast& e)
    {
        throw EDgiConfigError(
                "$parameter is missing, please check your timings config");
    }

