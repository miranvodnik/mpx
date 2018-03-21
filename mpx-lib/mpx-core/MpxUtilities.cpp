//    Event Driven Task Multiplexing Library
//    Copyright (C) 2018 Miran Vodnik
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
//    contact: miran.vodnik@siol.net

#include <mpx-core/MpxUtilities.h>

namespace mpx
{

/*! custom memory copy function
 *
 * official memcpy() function behaves unusually when the source and
 * destination fields overlap. Since mpx-lib always uses this function
 * by copying source field 'back' to the destination field, algorithm
 * used in this function does not corrupt source field, which is not
 * the case with the official version of this function.
 *
 * @param dest destination field
 * @param src source field
 * @param n number of bytes to copy from source to destination
 */
void MpxUtilities::memcpy (void* dest, const void* src, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		((u_char*) dest) [i] = (reinterpret_cast <const u_char*> (src)) [i];
}

} /* namespace mpx */
