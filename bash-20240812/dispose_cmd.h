/* dispose_cmd.h -- Functions appearing in dispose_cmd.c. */

/* Copyright (C) 1993-2023 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined (_DISPOSE_CMD_H_)
#define _DISPOSE_CMD_H_

#include "stdc.h"

extern void dispose_command (COMMAND *);
extern void dispose_word_desc (WORD_DESC *);
extern void dispose_word (WORD_DESC *);
extern void dispose_words (WORD_LIST *);
extern void dispose_word_array (char **);
extern void dispose_redirects (REDIRECT *);

#if defined (COND_COMMAND)
extern void dispose_cond_node (COND_COM *);
#endif

extern void dispose_function_def_contents (FUNCTION_DEF *);
extern void dispose_function_def (FUNCTION_DEF *);

extern void uw_dispose_command (void *);
extern void uw_dispose_words (void *);

#endif /* !_DISPOSE_CMD_H_ */
