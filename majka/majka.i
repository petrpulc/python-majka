%module Majka

%ignore fsa::find(const char * const sought, char * const results_buf, const char flags = 0);
%ignore fsa::results_count;

%rename (find) find_swig;

%typemap(out) char * fsa::find_swig {
	AV *myav;
	SV **svs;
	char * _result = $1;
	int i = 0, len = (int) ((arg1)->results_count);
	svs = (SV **) malloc(len*sizeof(SV *));
	for (i = 0; i < len ; i++) {
	    svs[i] = sv_newmortal();
	    int l = strlen(_result);
	    sv_setpvn((SV*)svs[i],_result,l);
	    _result += l + 1;
	};
	myav =	av_make(len,svs);
	free(svs);
        $result = newRV_noinc((SV*)myav);
        sv_2mortal($result);
        argvi++;
}

%{
#include "majka.h"
%}

%include "majka.h"

%perlcode %{

=cut
Usage:

perl -MMajka -Mv5.10 -e '
	my $m = Majka->new("majka.w-lt"); map say, @{$m->find("test")}'

Majka->new("path/to/data") creates the analyzer, $m->find($word, [flags])
returns a reference to an array of possible analyses. The optional second
parameter flags can be bitwise OR of $Majka::ADD_DIACRITICS (the analysis
restores possible diacritics) and $Majka::IGNORE_CASE (e.g. john would be
analyzed also as John).

$m->find($word, $buffer, [flags]) is thread-safe (and around 10 % slower)
variant of $m->find($word, [flags]). Example of use:

perl -MMajka -ne '
	BEGIN {
		$m = Majka->new("majka.w-lt");
		$buffer = " " x $m->{max_results_size};
		}
	chomp;
	print map "$_\n", @{$m->find($_, $buffer)};
	' < data

$m->{state} is equal to 0 if the automaton was successfully initialized,
non-zero otherwise.

=cut

package Majka;

sub new {
	Majka::fsa->new($_[1]);
	}

%}
