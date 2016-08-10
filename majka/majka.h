/* Based on Jan Daciuk's code from www.eti.pg.gda.pl/~jandac/fsa.html */

#define MAJKA_VERSION "Generated on 2016-01-22 from git version 1adf6a2 2015-03-26 (but changed)"

#define ADD_DIACRITICS		1
#define IGNORE_CASE		2
#define DISALLOW_LOWERCASE	4

const int max_word_length = 100; // in bytes

using namespace std;

inline size_t bytes2int(const unsigned char * addr, size_t len) {
  return *(size_t *) addr & ((size_t) -1 >> (sizeof(size_t) - len) * 8);
}

typedef	const unsigned char *	arc_pointer;

const int goto_offset = 1;

struct thread_specific {
  unsigned char *       candidate;
  unsigned char *       result;
  int                   results_count;
  size_t                input_len;
};

class fsa {
public:
  unsigned int		max_results_size;
#ifdef SWIG
  int			results_count;
#endif
  int			state;

  fsa(const char * const dict_name);
  int find(const char * const sought, char * const results_buf, const char flags = 0);
#ifdef SWIG
  char * find_swig(const char * const sought, const char flags = 0) { results_count = find(sought, results_buf, flags); return results_buf; }
  char * find_swig(const char * const sought, char * const buffer, const char flags = 0) { results_count = find(sought, buffer, flags); return buffer; }
  virtual ~fsa(void) { if (! state) { delete [] dict; delete [] results_buf; } }
#else
  virtual ~fsa(void) { if (! state) delete [] dict; }
#endif

private:
  arc_pointer	 	dict;
  unsigned char		type;
  int			goto_length;
  char			version_major;
  unsigned short int	version_minor;
  unsigned short int	max_result;
  unsigned short int	max_results_count;
  unsigned int		_max_results_size;
  size_t		input_len;
#ifdef SWIG
  char *		results_buf;
#endif
  arc_pointer		start;
  arc_pointer		start1, start2;
  unsigned char		table[3 * 256];
  unsigned char		tablelc[256];
#ifdef UTF
  unsigned char		table1[256], table2[256], table3[3][256];
#endif

  int read_fsa(const char * const dict_file_name);
  void find_word(const unsigned char * word, const int level, arc_pointer next_node, thread_specific &res);
  void accent_word(const unsigned char * const word, const int level, arc_pointer next_node, const arc_pointer start_node2, const unsigned char * accent_table, thread_specific &res);
  void compl_rest(const int depth, arc_pointer next_node, thread_specific &res);
  void process_result(thread_specific &res);

  arc_pointer first_node() const { return dict + goto_offset + goto_length; }
  arc_pointer set_next_node(const arc_pointer arc) const { return arc[goto_offset] & 4
    ? arc + goto_offset + 1
    : dict + (bytes2int(arc + goto_offset, goto_length) >> 3); }
  unsigned char get_letter(const arc_pointer arc) const { return *arc; }
  int is_final(const arc_pointer arc) const { return arc[goto_offset] & 1; }

void my_strcpy(unsigned char * &dest, const unsigned char * src) {
  size_t j = 0;
  for (size_t i = 0; src[i]; i++, j++)
#ifdef IL2
    dest[j] = src[i];
#else
    if (src[i] > 127) {
      dest[j] = table1[src[i]];
      dest[++j] = table2[src[i]];
    }
    else dest[j] = src[i];
#endif
  dest[j] = '\0';
  dest += j + 1;
}

void my_strxcpy(unsigned char * &dest, const unsigned char * src) {
  size_t j = 0, i = 0;
  for (; src[i] != ':'; i++, j++)
#ifdef IL2
    dest[j] = src[i];
#else
    if (src[i] > 127) {
      dest[j] = table1[src[i]];
      dest[++j] = table2[src[i]];
    }
    else dest[j] = src[i];
#endif
  src += i; dest += j;
  for (i = 0; src[i]; i++) dest[i] = src[i];
  dest[i] = '\0';
  dest += i + 1;
}

void my_strncpy(unsigned char * &dest, const unsigned char * src, size_t n) {
  size_t j = 0;
  for (size_t i = 0; i < n; i++, j++)
#ifdef IL2
    dest[j] = src[i];
#else
    if (src[i] > 127) {
      dest[j] = table1[src[i]];
      dest[++j] = table2[src[i]];
    }
    else dest[j] = src[i];
#endif
  dest += j;
}

};
