#include <string>

#include "iestaade.hpp"

struct MySettings {
    double init_p_acceptance = 0.1;
    size_t data_size         = 100;
    std::string date_format  = "!";
    double x                 = 1.0;
    size_t y                 = 2;
    std::string z            = "no config";

    MySettings() {}

    explicit MySettings(const std::string& config_filepath) :
        init_p_acceptance(iestaade::from_json<double>(config_filepath,
                                                      "run_engine/"
                                                      "simulated_annealing/"
                                                      "init_p_acceptance")),
        data_size(iestaade::from_json<size_t>(config_filepath,
                                              "run_engine/"
                                              "state/"
                                              "data_size")),
        date_format(iestaade::from_json<std::string>(config_filepath,
                                                     "run_engine/"
                                                     "report/"
                                                     "date_format"))
    {
    }
};

int main()
{
    MySettings s_config("examples/config.json");
    assert(s_config.init_p_acceptance == 0.97);
    assert(s_config.data_size == 200);
    assert(s_config.date_format == "%F");
    assert(s_config.x == 1.0);
    assert(s_config.y == 2);
    assert(s_config.z == "no config");

    MySettings s_no_config;
    assert(s_no_config.init_p_acceptance == 0.1);
    assert(s_no_config.data_size == 100);
    assert(s_no_config.date_format == "!");
    assert(s_no_config.x == 1.0);
    assert(s_no_config.y == 2);
    assert(s_no_config.z == "no config");

    return 0;
}
