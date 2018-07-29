/******************************************************************************
 * curlscript - https://github.com/xquery/curlscript
 ******************************************************************************
 * Copyright (c) 2017-2018 James Fuller <jim.fuller@webcomposite.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
******************************************************************************/

#include <curlscript/curlscript.cpp>
#include <cxxopts.hpp>

using namespace std;

int banner(){
    LOG_S(INFO) << "curlscript " << CURLSCRIPT_VERSION_MAJOR << "." << CURLSCRIPT_VERSION_MINOR << "." << CURLSCRIPT_VERSION_PATCH << "| Copyright 2018 James Fuller jim.fuller@webcomposite.com | https://github.com/xquery/curlscript";
    return EXIT_SUCCESS;
}

int usage(){
    LOG_S(INFO) << "> curlscript -d -l -s -q -f test.cs";
    return EXIT_SUCCESS;
}

cxxopts::Options setopts(){
    cxxopts::Options opts("curlscript", "http scripting with curl.");
    opts.add_options()
            ("positional",
             "Positional arguments: these are the arguments that are entered "
             "without an option", cxxopts::value<std::vector<std::string>>())
            ("d,debug", "Enable debugging")
            ("l,log", "Enable logging to file")
            ("s,serialiser", "Switch serialiser",cxxopts::value<string>())
            ("q,quiet", "Disable output")
            ("f,file", "File name", cxxopts::value<string>());
    opts.parse_positional({"input", "output", "positional"});
    return opts;
}

int main(int argc, char** argv ){

    DLOG_S(INFO) << "debug mode";
    DLOG_S(INFO) << "start processing...";

    cxxopts::Options opts = setopts();
    auto result = opts.parse(argc,argv);

    if(result["quiet"].count() == 1){
        DLOG_S(INFO) << "set quiet opt";
       set_log_verbosity_error();
    }else{
        DLOG_S(INFO) << "set emit INFO level";
        banner();
        set_log_verbosity_info(); }

    if(result["debug"].count() == 1){
        DLOG_S(INFO) << "set debug opt, emit maximum message level";
        set_log_verbosity_max(); }

    if(result["log"].count() == 1){
        DLOG_S(INFO) << "set logging to " << result["log"].as<string>();
        set_log_file(argc,argv,result["log"].as<string>()); }

    if(result["positional"].count() == 1){
        DLOG_S(INFO) << "set positional opt";
        auto& v = result["positional"].as<std::vector<std::string>>();
        for (const auto& s : v) {
            curlscript::eval(s);
        }
    } else{
        if(result["file"].count() == 1){
            DLOG_S(INFO) << "set file opt";
            curlscript::eval(result["file"].as<string>());
        }else{
            LOG_S(ERROR) << "must supply eval file.";
            return EXIT_FAILURE; }}

    DLOG_S(INFO) << "finished processing.";
    return EXIT_SUCCESS;
}
