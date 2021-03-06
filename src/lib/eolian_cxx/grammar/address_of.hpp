/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef EOLIAN_CXX_ADDRESS_OF_HH
#define EOLIAN_CXX_ADDRESS_OF_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/function_declaration.hpp"
#include "grammar/case.hpp"

namespace efl { namespace eolian { namespace grammar {

struct address_of_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::vector<std::string> cpp_namespaces = attributes::cpp_namespaces(cls.namespaces);

     if(!as_generator
        (
         scope_tab << " ::efl::eolian::address_of_operator<" << string
        ).generate(sink, cls.cxx_name, context)) return false;

     for(auto&& i : cls.inherits)
       {
         if(!as_generator(",  " << *("::" << lower_case[string]) << "::" << string)
            .generate(sink, std::make_tuple(attributes::cpp_namespaces(i.namespaces), i.eolian_name), context))
           return false;
       }

      if(!as_generator
         (
           "> operator&() { return {this}; }\n"
         ).generate(sink, attributes::unused, context)) return false;

     if(!as_generator
        (
         scope_tab << " ::efl::eolian::address_of_operator<" << string << " const "
        ).generate(sink, cls.cxx_name, context)) return false;

     for(auto&& i : cls.inherits)
       {
         if(!as_generator(",  " << *("::" << lower_case[string]) << "::" << string << " const ")
            .generate(sink, std::make_tuple(attributes::cpp_namespaces(i.namespaces), i.eolian_name), context))
           return false;
       }

      if(!as_generator
         (
           "> operator&() const { return {this}; }\n"
         ).generate(sink, attributes::unused, context)) return false;
      
      return true;
   }
};

template <>
struct is_eager_generator<address_of_generator> : std::true_type {};
template <>
struct is_generator<address_of_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<address_of_generator> : std::integral_constant<int, 1> {};
}
      
address_of_generator const address_of = {};
      
} } }

#endif
