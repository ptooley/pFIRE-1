//
//   Copyright 2019 University of Sheffield
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef ITERATOR_ROUTINES_HPP
#define ITERATOR_ROUTINES_HPP

template <typename Functor, typename OutputIterator, typename Input1, typename... Inputs>
OutputIterator
n_ary_transform(Functor f, OutputIterator out, Input1 first1, Input1 last1, Inputs... firsts)
{
  while (first1 != last1)
  {
    *out++ = f(*first1++, *firsts++...);
  }
  return out;
}

template <typename Functor, typename Input1, typename... Inputs>
Functor n_ary_for_each(Functor f, Input1 first1, Input1 last1, Inputs... firsts)
{
  while (first1 != last1)
  {
    f(*first1++, *firsts++...);
  }

  return f;
}

template <typename InputIt1, typename InputIt2, typename BinaryPredicate>
bool all_true(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, BinaryPredicate p)
{
  for (; first1 != last1 && first2 != last2; first1++, first2++)
  {
    if (!p(*first1, *first2))
    {
      return false;
    }
  }
  if (first1 != last1 || first2 != last2)
  {
    return false;
  }
  return true;
}

template <typename InputIt1, typename InputIt2, typename BinaryPredicate>
bool all_true_varlen(
    InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, BinaryPredicate p)
{
  for (; first1 != last1 && first2 != last2; first1++, first2++)
  {
    if (!p(*first1, *first2))
    {
      return false;
    }
  }
  return true;
}

#endif
