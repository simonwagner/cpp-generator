//
//  test_Generator.cpp
//  buffermanager
//
//  Created by Simon Wagner on 10.06.13.
//  Copyright (c) 2013 DBTUM. All rights reserved.
//

#include "generator.hpp"

#include <utility>
#include <iostream>

using namespace std;

GeneratorIterable<int> create_generator()
{
   auto c = [](Generator<int>& g)->void{
      for(int i = 0; i < 10; i++)
      {
         cout << "yield\t" << i << endl;
         g.yield(move(i));
      }
      cout << "exit\t" << endl;
   };
   
   return make_generator<int>(c);
}

int test_generator_main(int argc, char* argv[])
{
   GeneratorIterable<int> g = create_generator();
   
   for(auto i : g)
   {
      cout << "iterate\t" << i << endl;
   }
   return 0;
}
