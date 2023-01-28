/* { dg-do run } */
#ifndef THIS_SHOULD_NEVER_BE_DEFINED
'begin'
	'integer' x;
	x := 4;
	exit (x - 4);
'end';
#else
'causeerror';
#endif
/* and some more comments... */
