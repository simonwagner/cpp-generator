//
//  Generator.h
//  buffermanager
//
//  Created by Simon Wagner on 10.06.13.
//  Copyright (c) 2013 DBTUM. All rights reserved.
//

#ifndef __buffermanager__Generator__
#define __buffermanager__Generator__

#include <ucontext.h>
#include <functional>
#include <cassert>
#include <memory>

//typedef int GT;
enum GeneratorState {
   UNKNOWN, HAS_NEXT, END, INVALID
};

template<typename GT>
class Generator;

template<typename GT>
class GeneratorIterator {
   Generator<GT>& generator;
   int64_t index;
   GT* value;
public:
   static constexpr int64_t END_INDEX = -1;
private:
   bool next();
public:
   GeneratorIterator(Generator<GT>& generator, int64_t index);
   GeneratorIterator& operator++(); //++GeneratorIterator
   GeneratorIterator operator++(int); //GeneratorIterator++
   int64_t getIndex() { return index; }
   
   bool operator==(const GeneratorIterator<GT>& other) { return this->index == other.index; }
   bool operator!=(const GeneratorIterator<GT>& other) { return this->index != other.index; }
   GT& operator*() { assert(value); return *value; }
   GT* operator->() { assert(value); return value; }
};



template<typename GT>
class Generator {
public:
   typedef std::function<void(Generator<GT>&)> GeneratorRunCallback;
private:
   friend GeneratorIterator<GT>;
   GT* m_Next;
   GeneratorState m_state;
   ucontext_t m_mainContext;
   ucontext_t m_generatorContext;
   GeneratorRunCallback m_Run;
   void* m_stackData;
private:
   void take();
   static void generatorCallback(uint32_t upperHalf, uint32_t lowerHalf);
public:
   Generator(GeneratorRunCallback run);
   ~Generator();
   Generator(const Generator& g) = delete;
   Generator& operator=(const Generator& g) = delete;
   
   void yield(GT&& value);
   void exit();
   
   GT next();
   bool hasNext();
   
   GeneratorIterator<GT> begin() { return GeneratorIterator<GT>(*this, 0); };
   GeneratorIterator<GT> end() { return GeneratorIterator<GT>(*this, GeneratorIterator<GT>::END_INDEX); };
};

template<typename GT>
class GeneratorIterable {
   std::unique_ptr<Generator<GT>> generator;
   
public:
   GeneratorIterable(std::unique_ptr<Generator<GT>> generator) : generator(std::move(generator)) {}
   GeneratorIterable(GeneratorIterable&& i) : generator(std::move(i.generator)) {};
   GeneratorIterable(const GeneratorIterable& i) = delete;
   GeneratorIterable& operator=(const GeneratorIterable& i) = delete;
   
   GeneratorIterator<GT> begin() { return generator->begin(); };
   GeneratorIterator<GT> end() { return generator->end(); };
};

template<typename GT>
inline GeneratorIterable<GT> make_generator(typename Generator<GT>::GeneratorRunCallback run)
{
   return GeneratorIterable<GT>(std::unique_ptr<Generator<GT>>(new Generator<GT>(run)));
}

#include "generator.inl"

#endif /* defined(__buffermanager__Generator__) */
