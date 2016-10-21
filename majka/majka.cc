/* Based on Jan Daciuk's code from www.eti.pg.gda.pl/~jandac/fsa.html */

#include	<iostream>
#include	<fstream>
#include	<string.h>
#include	<stdlib.h>
#include	<new>
#include	"majka.h"

struct signature { // dictionary file signature
  char			sig[4];		// automaton identifier (magic number)
  char			ver;		// automaton type number (not used in fact)
  char			filler;		// char used as filler (not used)
  char			annot_sep;	// char that separates annotations from lex (not used)
  char			goto_length;	// length of go_to field
  char			type;
  char			version_major;
  unsigned short int	version_minor;
  unsigned short int	max_result;
  unsigned short int	max_results_count;
  unsigned int		max_results_size;
};

int fsa::read_fsa(const char * const dict_file_name) {
  const int	version = 5;
  streampos	file_ptr;
  long int	fsa_size;
  signature	sig_arc;

  // open dictionary file
  ifstream dict_file(dict_file_name, ios::in | ios::ate | ios::binary);
  if (dict_file.fail()) {
    cerr << "Cannot open dictionary file " << dict_file_name << endl;
    return 2;
  }
  fsa_size = (long int) dict_file.tellg() - sizeof(sig_arc);
  if (!dict_file.seekg(0L)) {
    cerr << "Seek on dictionary file " << dict_file_name << " failed" << endl;
    return 3;
  }

  // read and verify signature
  if (!(dict_file.read((char *)&sig_arc, sizeof(sig_arc)))) {
    cerr << "Cannot read a signature of dictionary file " << dict_file_name << endl;
    return 4;
  }
  if (strncmp(sig_arc.sig, "\\fsa", (size_t)4)) {
    cerr << "Invalid dictionary file (bad magic number): " << dict_file_name << endl;
    return 5;
  }
#define MYVERSION 1
  if (sig_arc.version_major != MYVERSION) {
    cerr << "Invalid majka dictionary version (" << sig_arc.version_major << " instead of " << MYVERSION << ") "
         << "of dictionary file " << dict_file_name << endl;
    return 6;
  }
  version_major = sig_arc.version_major;
  if (sig_arc.ver != version) {
    cerr << "Invalid fsa dictionary version (" << int(sig_arc.ver) << " instead of 5) "
         << "of dictionary file " << dict_file_name << endl;
    return 61;
  }
  max_result		= sig_arc.max_result;
  max_results_count	= sig_arc.max_results_count;
  _max_results_size	= sig_arc.max_results_size;
  max_results_size	= _max_results_size + 2 * (max_word_length + 2);
  type			= sig_arc.type;
  version_minor		= sig_arc.version_minor;
  goto_length		= sig_arc.goto_length & 0x0f;

  // allocate memory and read the automaton, + sizeof(size_t) due to bytes2int :-)
  dict = new unsigned char[fsa_size + sizeof(size_t)];
  if (!(dict_file.read((char *) dict, fsa_size))) {
    cerr << "Cannot read dictionary file " << dict_file_name << endl;
    delete [] dict;
    return 7;
  }
  return 0;
}

#define forallnodes(node, i) for (int i = 1; i; i = !(node[goto_offset] & 2), node += goto_offset + goto_length)

fsa::fsa(const char * const dict_name) {
  if ((state = read_fsa(dict_name))) return;

#ifdef SWIG
  results_buf = new char[max_results_size];

#endif
  start = first_node();
  start1 = start2 = NULL;

  arc_pointer next_node;
  next_node = set_next_node(start);
  forallnodes(next_node, i) {
    if ('!' == get_letter(next_node)) {
      start1 = next_node;
      break;
      }
    }
  next_node = set_next_node(start);
  forallnodes(next_node, j) {
    if ('^' == get_letter(next_node)) {
      start2 = next_node;
      break;
      }
    }

  for (int i = 0; i < 256; i++) table[i] = i;
  table[161] = 'A'; table[177] = 'a'; // Ąą
  table[163] = 'L'; table[179] = 'l'; // Łł
  table[165] = 'L'; table[181] = 'l'; // Ľľ
  table[166] = 'S'; table[182] = 's'; // Śś
  table[169] = 'S'; table[185] = 's'; // Šš
  table[170] = 'S'; table[186] = 's'; // Şş
  table[171] = 'T'; table[187] = 't'; // Ťť
  table[172] = 'Z'; table[188] = 'z'; // Źź
  table[174] = 'Z'; table[190] = 'z'; // Žž
  table[175] = 'Z'; table[191] = 'z'; // Żż
  table[192] = 'R'; table[224] = 'r'; // Ŕŕ
  table[193] = 'A'; table[225] = 'a'; // Áá
  table[194] = 'A'; table[226] = 'a'; // Ââ
  table[195] = 'A'; table[227] = 'a'; // Ăă
  table[196] = 'A'; table[228] = 'a'; // Ää
  table[197] = 'L'; table[229] = 'l'; // Ĺĺ
  table[198] = 'C'; table[230] = 'c'; // Ćć
  table[199] = 'C'; table[231] = 'c'; // Çç
  table[200] = 'C'; table[232] = 'c'; // Čč
  table[201] = 'E'; table[233] = 'e'; // Éé
  table[202] = 'E'; table[234] = 'e'; // Ęę
  table[203] = 'E'; table[235] = 'e'; // Ëë
  table[204] = 'E'; table[236] = 'e'; // Ěě
  table[205] = 'I'; table[237] = 'i'; // Íí
  table[206] = 'I'; table[238] = 'i'; // Îî
  table[207] = 'D'; table[239] = 'd'; // Ďď
  table[208] = 'D'; table[240] = 'd'; // Đđ
  table[209] = 'N'; table[241] = 'n'; // Ńń
  table[210] = 'N'; table[242] = 'n'; // Ňň
  table[211] = 'O'; table[243] = 'o'; // Óó
  table[212] = 'O'; table[244] = 'o'; // Ôô
  table[213] = 'O'; table[245] = 'o'; // Őő
  table[214] = 'O'; table[246] = 'o'; // Öö
  table[216] = 'R'; table[248] = 'r'; // Řř
  table[217] = 'U'; table[249] = 'u'; // Ůů
  table[218] = 'U'; table[250] = 'u'; // Úú
  table[219] = 'U'; table[251] = 'u'; // Űű
  table[220] = 'U'; table[252] = 'u'; // Üü
  table[221] = 'Y'; table[253] = 'y'; // Ýý
  table[222] = 'T'; table[254] = 't'; // Ţţ

  for (int i = 0; i < 256; i++) table[256 + i] = tablelc[i] = i >= 'A' && i <= 'Z' ? i + 'a' - 'A' : i;
  for (int i = 161; i < 176; i++) table[256 + i] = tablelc[i] = i + 16;
  for (int i = 192; i < 223; i++) table[256 + i] = tablelc[i] = i + 32;

  for (int i = 0; i < 256; i++) table[512 + i] = table[table[256 + i]];
#ifdef UTF

  // UTF-8 <-> ISO-8859-2
  for (int i = 0; i < 256; i++) table1[i] = table2[i] = table3[0][i] = table3[1][i] = table3[2][i] = 32;
  table1[161] = 196; table2[161] = 132; table1[177] = 196; table2[177] = 133; // Ąą
  table1[163] = 197; table2[163] = 129; table1[179] = 197; table2[179] = 130; // Łł
  table1[165] = 196; table2[165] = 189; table1[181] = 196; table2[181] = 190; // Ľľ
  table1[166] = 197; table2[166] = 154; table1[182] = 197; table2[182] = 155; // Śś
  table1[169] = 197; table2[169] = 160; table1[185] = 197; table2[185] = 161; // Šš
  table1[170] = 197; table2[170] = 158; table1[186] = 197; table2[186] = 159; // Şş
  table1[171] = 197; table2[171] = 164; table1[187] = 197; table2[187] = 165; // Ťť
  table1[172] = 197; table2[172] = 185; table1[188] = 197; table2[188] = 186; // Źź
  table1[174] = 197; table2[174] = 189; table1[190] = 197; table2[190] = 190; // Žž
  table1[175] = 197; table2[175] = 187; table1[191] = 197; table2[191] = 188; // Żż
  table1[192] = 197; table2[192] = 148; table1[224] = 197; table2[224] = 149; // Ŕŕ
  table1[193] = 195; table2[193] = 129; table1[225] = 195; table2[225] = 161; // Áá
  table1[194] = 195; table2[194] = 130; table1[226] = 195; table2[226] = 162; // Ââ
  table1[195] = 196; table2[195] = 130; table1[227] = 196; table2[227] = 131; // Ăă
  table1[196] = 195; table2[196] = 132; table1[228] = 195; table2[228] = 164; // Ää
  table1[197] = 196; table2[197] = 185; table1[229] = 196; table2[229] = 186; // Ĺĺ
  table1[198] = 196; table2[198] = 134; table1[230] = 196; table2[230] = 135; // Ćć
  table1[199] = 195; table2[199] = 135; table1[231] = 195; table2[231] = 167; // Çç
  table1[200] = 196; table2[200] = 140; table1[232] = 196; table2[232] = 141; // Čč
  table1[201] = 195; table2[201] = 137; table1[233] = 195; table2[233] = 169; // Éé
  table1[202] = 196; table2[202] = 152; table1[234] = 196; table2[234] = 153; // Ęę
  table1[203] = 195; table2[203] = 139; table1[235] = 195; table2[235] = 171; // Ëë
  table1[204] = 196; table2[204] = 154; table1[236] = 196; table2[236] = 155; // Ěě
  table1[205] = 195; table2[205] = 141; table1[237] = 195; table2[237] = 173; // Íí
  table1[206] = 195; table2[206] = 142; table1[238] = 195; table2[238] = 174; // Îî
  table1[207] = 196; table2[207] = 142; table1[239] = 196; table2[239] = 143; // Ďď
  table1[208] = 195; table2[208] = 144; table1[240] = 195; table2[240] = 176; // Đđ
  table1[209] = 197; table2[209] = 131; table1[241] = 197; table2[241] = 132; // Ńń
  table1[210] = 197; table2[210] = 135; table1[242] = 197; table2[242] = 136; // Ňň
  table1[211] = 195; table2[211] = 147; table1[243] = 195; table2[243] = 179; // Óó
  table1[212] = 195; table2[212] = 148; table1[244] = 195; table2[244] = 180; // Ôô
  table1[213] = 197; table2[213] = 144; table1[245] = 197; table2[245] = 145; // Őő
  table1[214] = 195; table2[214] = 150; table1[246] = 195; table2[246] = 182; // Öö
  table1[216] = 197; table2[216] = 152; table1[248] = 197; table2[248] = 153; // Řř
  table1[217] = 197; table2[217] = 174; table1[249] = 197; table2[249] = 175; // Ůů
  table1[218] = 195; table2[218] = 154; table1[250] = 195; table2[250] = 186; // Úú
  table1[219] = 197; table2[219] = 176; table1[251] = 197; table2[251] = 177; // Űű
  table1[220] = 195; table2[220] = 156; table1[252] = 195; table2[252] = 188; // Üü
  table1[221] = 195; table2[221] = 157; table1[253] = 195; table2[253] = 189; // Ýý
  table1[222] = 197; table2[222] = 162; table1[254] = 197; table2[254] = 163; // Ţţ

  table3[1][132] = 161; table3[1][133] = 177; // Ąą
  table3[2][129] = 163; table3[2][130] = 179; // Łł
  table3[1][189] = 165; table3[1][190] = 181; // Ľľ
  table3[2][154] = 166; table3[2][155] = 182; // Śś
  table3[2][160] = 169; table3[2][161] = 185; // Šš
  table3[2][158] = 170; table3[2][159] = 186; // Şş
  table3[2][164] = 171; table3[2][165] = 187; // Ťť
  table3[2][185] = 172; table3[2][186] = 188; // Źź
  table3[2][189] = 174; table3[2][190] = 190; // Žž
  table3[2][187] = 175; table3[2][188] = 191; // Żż
  table3[2][148] = 192; table3[2][149] = 224; // Ŕŕ
  table3[0][129] = 193; table3[0][161] = 225; // Áá
  table3[0][130] = 194; table3[0][162] = 226; // Ââ
  table3[1][130] = 195; table3[1][131] = 227; // Ăă
  table3[0][132] = 196; table3[0][164] = 228; // Ää
  table3[1][185] = 197; table3[1][186] = 229; // Ĺĺ
  table3[1][134] = 198; table3[1][135] = 230; // Ćć
  table3[0][135] = 199; table3[0][167] = 231; // Çç
  table3[1][140] = 200; table3[1][141] = 232; // Čč
  table3[0][137] = 201; table3[0][169] = 233; // Éé
  table3[1][152] = 202; table3[1][153] = 234; // Ęę
  table3[0][139] = 203; table3[0][171] = 235; // Ëë
  table3[1][154] = 204; table3[1][155] = 236; // Ěě
  table3[0][141] = 205; table3[0][173] = 237; // Íí
  table3[0][142] = 206; table3[0][174] = 238; // Îî
  table3[1][142] = 207; table3[1][143] = 239; // Ďď
  table3[0][144] = 208; table3[0][176] = 240; // Đđ
  table3[2][131] = 209; table3[2][132] = 241; // Ńń
  table3[2][135] = 210; table3[2][136] = 242; // Ňň
  table3[0][147] = 211; table3[0][179] = 243; // Óó
  table3[0][148] = 212; table3[0][180] = 244; // Ôô
  table3[2][144] = 213; table3[2][145] = 245; // Őő
  table3[0][150] = 214; table3[0][182] = 246; // Öö
  table3[2][152] = 216; table3[2][153] = 248; // Řř
  table3[2][174] = 217; table3[2][175] = 249; // Ůů
  table3[0][154] = 218; table3[0][186] = 250; // Úú
  table3[2][176] = 219; table3[2][177] = 251; // Űű
  table3[0][156] = 220; table3[0][188] = 252; // Üü
  table3[0][157] = 221; table3[0][189] = 253; // Ýý
  table3[2][162] = 222; table3[2][163] = 254; // Ţţ
#endif
}

// Rather ugly temporary solution...
#define candidate res.candidate
#define result res.result
#define results_count res.results_count
#define input_len res.input_len
int fsa::find(const char * const sought, char * const results_buf, const char flags) {
  unsigned char * copy = (unsigned char *) results_buf + _max_results_size;
  thread_specific res;

  candidate = (unsigned char *) results_buf + _max_results_size + max_word_length + 2;
  result = (unsigned char *) results_buf;
  results_count = 0;

  unsigned char * j = copy;
  const unsigned char * tmp = (const unsigned char *) sought + max_word_length;
  char uppercase = 0;
  for (const unsigned char * i = (const unsigned char *) sought; *i && i < tmp; i++, j++) {
#ifdef IL2
    *j = *i;
#else
    if (128 > *i) *j = *i;
    else if (*i > 194 && *i < 198) {
      *j = table3[*i - 195][*(i + 1)];
      i++;
    }
    else return results_count;
#endif
    if (! (flags & (IGNORE_CASE | DISALLOW_LOWERCASE)) && j != copy && tablelc[*j] != *j) uppercase = 1;
  }
  input_len = j - copy;
  *j = ':';
  *(j + 1) = '\0';

  if (flags & (ADD_DIACRITICS | IGNORE_CASE)) {
    const unsigned char * accent_table = table + 256 * (flags - 1);
    if (flags & IGNORE_CASE) for (unsigned char * i = copy; *i; i++) *i = tablelc[*i];
    accent_word(copy, 0, start, NULL, accent_table, res);
    if (uppercase) {
      for (unsigned char * i = (copy + 1); *i; i++) *i = tablelc[*i];
      accent_word(copy, 0, start, NULL, accent_table, res);
    }
    if (tablelc[*copy] != *copy) {
      *copy = tablelc[*copy];
      accent_word(copy, 0, start, NULL, accent_table, res);
    }
    if ((! results_count) && start1 && start2) accent_word(copy, 0, start1, start2, accent_table, res);
  }
  else {
    find_word(copy, 0, start, res);
    if (uppercase) {
      for (unsigned char * i = (copy + 1); *i; i++) *i = tablelc[*i];
      find_word(copy, 0, start, res);
    }
    if (tablelc[*copy] != *copy && ! (flags & DISALLOW_LOWERCASE)) {
      *copy = tablelc[*copy];
      find_word(copy, 0, start, res);
    }
    if (! results_count && start1 && start2) {
      arc_pointer new_node, next_node = set_next_node(start1);
      int level = 0;
      bool found = false;
      unsigned char * word = copy;

      do {
        found = false;
        forallnodes(next_node, i)
          if (*word == get_letter(next_node)) {
            candidate[level++] = get_letter(next_node);
            if (*++word == '\0') return results_count;
            found = true;
            new_node = next_node = set_next_node(next_node);
            break;
          }
        if (found) forallnodes(new_node, j)
          if (':' == get_letter(new_node)) {
            find_word(word, level, start2, res);
            break;
          }
      } while (found);
    }
  }
  return results_count;
}

void fsa::accent_word(const unsigned char * const word, const int level, arc_pointer next_node, const arc_pointer start_node2, const unsigned char * accent_table, thread_specific &res) {
  next_node = set_next_node(next_node);
  unsigned char	char_no;
  forallnodes(next_node, i) {
    char_no = get_letter(next_node);
    if (*word == char_no || *word == accent_table[char_no]) {
      candidate[level] = get_letter(next_node);
      if (word[1] == '\0' && ! start_node2) compl_rest(level + 1, next_node, res);
      else accent_word(word + 1, level + 1, next_node, start_node2, accent_table, res);
    }
    else if (get_letter(next_node) == ':' && start_node2) accent_word(word, level, start_node2, NULL, accent_table, res);
  }
}

void fsa::find_word(const unsigned char * word, int level, arc_pointer next_node, thread_specific &res) {
  next_node = set_next_node(next_node);
  bool found;
  do {
    found = false;
    forallnodes(next_node, i) {
      if (*word == get_letter(next_node)) {
        candidate[level++] = get_letter(next_node);
	if (word[1] == '\0') compl_rest(level, next_node, res); else found = ++word;
	break;
      }
    }
    if (found) next_node = set_next_node(next_node);
  } while (found);
}

void fsa::compl_rest(const int depth, arc_pointer next_node, thread_specific &res) {
  next_node = set_next_node(next_node);
  if (next_node == dict) return;
  forallnodes(next_node, i) {
    candidate[depth] = get_letter(next_node);
    if (is_final(next_node)) {
      candidate[depth + 1] = '\0';
      process_result(res);
      results_count++;
    }
    compl_rest(depth + 1, next_node, res);
  }
}

void fsa::process_result(thread_specific &res) { switch (type) { // not indented

case 1:   // w-lt
case 4: { // l-wt
  my_strncpy(result, candidate, input_len - (candidate[input_len + 1] - 'A'));
  my_strxcpy(result, candidate + input_len + 2);
} break;

case 3: { // lt-w
  // We want to allow l-tw queries also (we would have to check a presence of [the "first"] ':' in the query otherwise)
  const unsigned char * const first = (unsigned char *) strchr((char *) candidate, ':');
  const unsigned char * second = candidate + input_len;
  if (first == second) {
    second = (unsigned char *) strchr((char *) first + 1, ':');
    my_strncpy(result, first + 1, second - first);
    }
  my_strncpy(result, candidate, first - candidate - (second[1] - 'A'));
  my_strcpy(result, second + 2);
} break;

case 2:	  // w
case 5:   // l-w
case 6:   // w-l
case 7: { // w-w
  my_strncpy(result, candidate, input_len - (candidate[input_len + 1] - 'A'));
  my_strcpy(result, candidate + input_len + 2);
} break;

case 1 + 128: { // w-lt
  int prefix_len = candidate[input_len + 1] - 'A';
  int n = input_len - prefix_len;
  n -= candidate[input_len + 2] - 'A';
  if (n<0) n = 0;
  my_strncpy(result, candidate + prefix_len, (size_t) n);
  my_strxcpy(result, candidate + input_len + 3);
} break;

case 2 + 128: { // w
  my_strncpy(result, candidate, input_len);
  *result++ = '\0';
} break;

case 3 + 128: { // lt-w
  const unsigned char * const first = (unsigned char *) strchr((char *) candidate, ':');
  const unsigned char * second = candidate + input_len;
  if (first == second) {
    second = (unsigned char *) strchr((char *) first + 1, ':');
    my_strncpy(result, first + 1, second - first);
    }
  int prefix_len = second[1] - 'A';
  my_strncpy(result, second + 2, prefix_len);
  my_strncpy(result, candidate, first - candidate - (second[prefix_len + 2] - 'A'));
  my_strcpy(result, second + prefix_len + 3);
} break;

case 4 + 128: { // l-wt
  int prefix_len = candidate[input_len + 1] - 'A';
  my_strncpy(result, candidate + input_len + 2, prefix_len);
  my_strncpy(result, candidate, input_len - (candidate[input_len + prefix_len + 2] - 'A'));
  my_strxcpy(result, candidate + input_len + prefix_len + 3);
} break;

case 5 + 128: { // l-w
  int prefix_len = candidate[input_len + 1] - 'A';
  my_strncpy(result, candidate + input_len + 2, prefix_len);
  my_strncpy(result, candidate, input_len - (candidate[input_len + prefix_len + 2] - 'A'));
  my_strcpy(result, candidate + input_len + prefix_len + 3);
} break;

case 6 + 128: { // w-l
  int prefix_len = candidate[input_len + 1] - 'A';
  my_strncpy(result, candidate + prefix_len, input_len - prefix_len - (candidate[input_len + 2] - 'A'));
  my_strcpy(result, candidate + input_len + 3);
} break;

case 7 + 128: { // w-w
  int prefix_add_len = candidate[input_len + 1] - 'A';
  my_strncpy(result, candidate + input_len + 2, prefix_add_len);
  int prefix_remove_len = candidate[input_len + 2 + prefix_add_len] - 'A';
  my_strncpy(result, candidate + prefix_remove_len, input_len - prefix_remove_len - (candidate[input_len + 3 + prefix_add_len] - 'A'));
  my_strcpy(result, candidate + input_len + prefix_add_len + 4);
} break;

default:
  cerr << "Invalid dictionary file (cannot interpret file of type " << (short int) type << ")" << endl;
  // Yes, exiting from a library is rather ugly, but it should not happen and this is the simplest solution
  exit(EXIT_FAILURE); // And of course it does not free the memory, but again, this should not happen :-)
}}
