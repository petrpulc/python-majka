#include	<iostream>
#include	<string.h>
#include	<stdlib.h>
#include	<new>
#include	"majka.h"

int main(const int argc, const char *argv[]) {
  int print = 0;
  int flags = 0;
  char data[255] = "";

  for (int i = 1; i < argc; i++) {
    if (! strcmp(argv[i], "-f") && ++i < argc) strcpy(data, argv[i]);
    if (! strcmp(argv[i], "-p")) print = 1;
    if (! strcmp(argv[i], "-d")) flags |= ADD_DIACRITICS;
    if (! strcmp(argv[i], "-i")) flags |= IGNORE_CASE;
    if (! strcmp(argv[i], "-l")) flags |= DISALLOW_LOWERCASE;
    if (! strcmp(argv[i], "-h")) {
      cerr << MAJKA_VERSION << endl;
      cerr << "-f file  dictionary file" << endl
           << "-p       copy the input word to the output and output the results as one line" << endl
           << "-d       add diacritics" << endl
           << "-i       ignore case (analyze john as John; Dog/DOG is always analyzed as dog unless -l)" << endl
           << "-l       do NOT lowercase (analyze JOHN as John or Dog/DOG as dog)" << endl
           << "-h       help" << endl;
      return 0;
      }
    }
  if (! *data) {
    cerr << "Missing dictionary file (-f option)" << endl;
    return 1;
    }

  fsa majka(data);
  if (majka.state) return majka.state;

  char * results = new char[majka.max_results_size];
  char * word = new char[max_word_length + 1];
  const char * result;
  int i, rc;

  while (! cin.eof()) {
    cin.getline(word, max_word_length + 1);
    if (! word[0] && cin.eof()) break;
    if (cin.rdstate() == ios::failbit) {
      while (cin.rdstate() == ios::failbit) {
        if (print) cout << word;
        cin.clear();
        cin.getline(word, max_word_length + 1);
        }
      if (print) cout << word << "\n";
      }
    else {
      if (print) cout << word;
      rc = majka.find(word, results, flags);
      for (result = results, i = 0; i < rc; i++, result += strlen(result) + 1)
        if (print) cout << ":" << result; else cout << result << "\n";
      if (print) cout << "\n";
    }
  }
  delete [] results;
  delete [] word;
}
