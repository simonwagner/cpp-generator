//
//  Generator.cpp
//  buffermanager
//
//  Created by Simon Wagner on 10.06.13.
//  Copyright (c) 2013 DBTUM. All rights reserved.
//

#include "Generator.hpp"

#include <ucontext.h>
#include <cassert>
#include <utility>
#include <iostream>

using namespace std;

template<typename GT>
GeneratorIterator<GT>::GeneratorIterator(Generator<GT>& generator, int64_t index)
   : generator(generator), index(index)
{
   value = nullptr;
   // fetch the first value after initialisation
   //might not fetch anythin, if no data is available
   next();
}

template<typename GT>
bool GeneratorIterator<GT>::next()
{
   if(index == END_INDEX)
   {
      return false; //if we are already at the end, do not fetch the next item
   }

   //fetch the next item
   bool hasNext = generator.hasNext();
   if(!hasNext)
   {
      //if we do not have a next item, set the index to END_INDEX
      index = END_INDEX;
      return false;
   }
   else
   {
      //if we do have an valid index fetch value from generator
      //very inefficent, has someone a better idea how to handle this
      //without allocating new objects each time we iterate
      value = new GT(move(generator.next()));
      return true;
   }
}

template<typename GT>
GeneratorIterator<GT>& GeneratorIterator<GT>::operator++() //++GeneratorIterator
{
   if(next()) index++; //advance the index, if we have fetched a new value
   return *this;
}

template<typename GT>
GeneratorIterator<GT> GeneratorIterator<GT>::operator++(int) //GeneratorIterator++
{
   GeneratorIterator tmp(*this);
   ++*this;
   return tmp;
}

template<typename GT>
Generator<GT>::Generator(GeneratorRunCallback run)
    : m_Run(run), m_state(GeneratorState::UNKNOWN)
{
   //get the current context as starting point for the generator context
   getcontext(&m_generatorContext);
   //calculate the arguments for the generatorCallback
   uint64_t generatorAdress = (uint64_t)this;
   uint32_t upperHalf = (uint32_t)(generatorAdress >> 32);
   uint32_t lowerHalf = (uint32_t)(generatorAdress & 0xFFFFFFFF);
   //allocate stack for context
   m_stackData = malloc(SIGSTKSZ);
   stack_t stack;
   stack.ss_size = SIGSTKSZ;
   stack.ss_sp = m_stackData;
   stack.ss_flags = 0;
   m_generatorContext.uc_stack = stack;
   //create the context
   makecontext(&m_generatorContext, (void(*)())&(Generator::generatorCallback), 2, upperHalf, lowerHalf);
}

template<typename GT>
Generator<GT>::~Generator()
{
   free(m_stackData);
}

template<typename GT>
void Generator<GT>::generatorCallback(uint32_t upperHalf, uint32_t lowerHalf)
{
   uint64_t generatorAdress = (uint64_t)upperHalf << 32 | lowerHalf;
   Generator* generator = (Generator*)(generatorAdress);
   generator->m_Run(*generator);
   generator->exit();
}

template<typename GT>
void Generator<GT>::yield(GT&& value)
{
   m_Next = new GT(move(value));
   m_state = GeneratorState::HAS_NEXT;
   swapcontext(&m_generatorContext, &m_mainContext);
}

template<typename GT>
void Generator<GT>::exit()
{
   if(m_state != GeneratorState::END)
   {
      m_state = GeneratorState::END;
      setcontext(&m_mainContext);
   }
}

template<typename GT>
GT Generator<GT>::next()
{
   if(m_state == GeneratorState::UNKNOWN)
   {
      take();
   }
   assert(m_state == GeneratorState::HAS_NEXT);
   m_state = GeneratorState::UNKNOWN;
   
   GT&& return_value = move(*(this->m_Next));
   delete this->m_Next;
   return return_value;
}

template<typename GT>
bool Generator<GT>::hasNext()
{
   if(m_state == GeneratorState::UNKNOWN)
   {
      take();
   }
   
   return m_state == GeneratorState::HAS_NEXT;
}

template<typename GT>
void Generator<GT>::take()
{
   assert(m_state == GeneratorState::UNKNOWN);
   swapcontext(&m_mainContext, &m_generatorContext);
   assert(m_state == GeneratorState::HAS_NEXT || m_state == GeneratorState::END);
}