/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

 #include "test.h"

 #include <gflags/gflags.h>
 #include <glog/logging.h>

 // gflags options
 DECLARE_bool(help);
 DECLARE_bool(helpshort);
 DECLARE_string(helpmatch);
 DECLARE_double(rtfactor);
 DEFINE_double(
     rtfactor,
     -1,
     "real-time factor (speed) (-1: unlimitted/fastest)");

 namespace {

 // lest options
 DEFINE_bool(abort, false, "abort at first failure");
 DEFINE_bool(count, false, "count selected tests");
 DEFINE_bool(list_tags, false, "list tags of selected tests");
 DEFINE_bool(list_tests, false, "list selected tests");
 DEFINE_bool(pass, false, "also report passing tests");
 DEFINE_bool(time, false, "list duration of selected tests");
 DEFINE_string(
     order,
     "declared",
     "test order ('declared', 'lexical' or 'random')");
 DEFINE_uint64(seed, 0, "use n for random generator seed");
 DEFINE_int32(repeat, 1, "repeat selected tests n times (-1: indefinite)");
 DEFINE_bool(logsinktostderr, true, "Log sink to stderror.");
 DEFINE_string(logsinkdir, "", "Optional directory to write sink log files");

 // Set up lest options from gflags
 std::tuple<lest::options, lest::texts> parseLestArguments(
     int argc,
     char** argv) {
   lest::options option;
   option.abort = FLAGS_abort;
   option.count = FLAGS_count;
   option.tags = FLAGS_list_tags;
   option.list = FLAGS_list_tests;
   option.pass = FLAGS_pass;
   option.time = FLAGS_time;
   option.seed = FLAGS_seed;
   option.repeat = FLAGS_repeat;
   if (FLAGS_order == "lexical") {
     option.lexical = true;
   } else if (FLAGS_order == "random") {
     option.random = true;
   } else if (FLAGS_order != "declared") {
     throw std::runtime_error("Unknown test order " + FLAGS_order);
   }

   lest::texts in;
   for (int i = 1; i < argc; i++) {
     in.push_back(argv[i]);
   }

   return std::make_tuple(option, in);
 }

 // Port of lest::run for explicit arguments
 int runLest(
     lest::tests specification,
     std::tuple<lest::options, lest::texts> opts) {
   std::ostream& os = std::cout;
   try {
     auto option = std::get<0>(opts);
     auto in = std::get<1>(opts);

     if (option.lexical) {
       lest::sort(specification);
     }
     if (option.random) {
       lest::shuffle(specification, option);
     }

     if (option.count) {
       return lest::for_test(specification, in, lest::count(os));
     }
     if (option.list) {
       return lest::for_test(specification, in, lest::print(os));
     }
     if (option.tags) {
       return lest::for_test(specification, in, lest::ptags(os));
     }
     if (option.time) {
       return lest::for_test(specification, in, lest::times(os, option));
     }

     return lest::for_test(
         specification, in, lest::confirm(os, option), option.repeat);
   } catch (std::exception const& e) {
     std::cerr << "Error: " << e.what() << "\n";
     return 1;
   }
 }

 } // namespace

 lest::tests& specification() {
   static lest::tests tests;
   return tests;
 }

 int main(int argc, char* argv[]) {
   FLAGS_minloglevel = google::ERROR;
   google::InitGoogleLogging(argv[0]);

   gflags::SetUsageMessage(
       "[options] [test-spec ...]\n\n"
       "  Test specification:\n"
       "    \"@\", \"*\" all tests, unless excluded\n"
       "    empty    all tests, unless tagged [hide] or [.optional-name]\n"
 #if lest_FEATURE_REGEX_SEARCH
       "    \"re\"     select tests that match regular expression\n"
       "    \"!re\"    omit tests that match regular expression"
 #else
       "    \"text\"   select tests that contain text (case insensitive)\n"
       "    \"!text\"  omit tests that contain text (case insensitive)"
 #endif
   );

   // Limit help output to relevant flags only
   gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);
   if (FLAGS_help) {
     if (FLAGS_helpmatch.empty()) {
       FLAGS_helpmatch = "main_test";
       FLAGS_help = false;
     }
   }
   gflags::HandleCommandLineHelpFlags();

   // Setup lest options
   int status = runLest(specification(), parseLestArguments(argc, argv));
   
   return status;
 }
