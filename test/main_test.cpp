/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

 #include "test.h"

 namespace {

 // Set up lest options from gflags
 std::tuple<lest::options, lest::texts> parseLestArguments(
     int argc,
     char** argv) {
   lest::options option;
   option.abort = false; //abort at first failure
   option.count = false; //count selected tests;
   option.tags = false; //list tags of selected tests
   option.list = false; //list selected tests
   option.pass = false; //also report passing tests
   option.time = false; //list duration of selected tests
   option.seed = 0; //RNG seed
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
   
   // Setup lest options
   int status = runLest(specification(), parseLestArguments(argc, argv));

   return status;
 }
