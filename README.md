#cpp-generator

Generators for C++, using ucontext to switch context

Allows you to use yield like in Python. Makes iterators in C++ a bit bearable

##Example usage

```cpp
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
```
