/* Copyright, under GPL 2.0 2016 <petr.pulc@wolterskluwer.com> */

#include <Python.h>
#include <structmember.h>
#include <string.h>
#include <iostream>
#include "majka/majka.h"

#if PY_MAJOR_VERSION >= 3
  #define PY3K
#endif

typedef struct {
  PyObject_HEAD
  fsa* majka;
  int flags;
  bool tags;
  bool compact_tag;
  bool first_only;
  PyObject* negative;
} Majka;

static void Majka_dealloc(Majka* self) {
  delete self->majka;
  Py_DECREF(self->negative);
  Py_TYPE(self)->tp_free(reinterpret_cast<Majka*>(self));
}

static PyObject* Majka_new(PyTypeObject* type,
                           PyObject* args,
                           PyObject* kwds) {
  Majka* self;
  self = reinterpret_cast<Majka*>(type->tp_alloc(type, 0));
  self->flags = 0;
  self->tags = true;
  self->compact_tag = false;
  self->first_only = false;
  self->negative = PyUnicode_FromString("-");
  return reinterpret_cast<PyObject*>(self);
}

static int Majka_init(Majka* self, PyObject* args, PyObject* kwds) {
  const char* file = NULL;
  static char* kwlist[] = {const_cast<char*>("file"), NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &file)) {
    return -1;
  }

  if (!file) {
      PyErr_SetString(PyExc_TypeError,
                      "Majka initialization needs a path to dictionary");
    return -1;
  }

  self->majka = new fsa(file);

  if (self->majka->state) {
      PyErr_SetString(PyExc_IOError,
                      "Majka dictionary is unreadable or invalid");
    return -1;
  }

  return 0;
}

int is_negation(const char* tag_string){
  if (*tag_string == 'k'){
    tag_string += 2;
  }
  if (*tag_string == 'e') {
    ++tag_string;
    if (*tag_string == 'N') {
      return 1;
    }
  }
  return 0;
}

static int dict_set(PyObject* dict, const char* key, PyObject* obj){
  int rv = PyDict_SetItemString(dict, key, obj);
  Py_DECREF(obj);
  return rv;
}

static int dict_set_string(PyObject* dict, const char* key, const char* val){
  PyObject* obj = PyUnicode_FromString(val);
  return dict_set(dict, key, obj);
}

static int dict_set_numeric(PyObject* dict, const char* key, char* val){
  PyObject* obj = PyLong_FromString(val, NULL, 10);
  return dict_set(dict, key, obj);
}

static int list_append(PyObject* list, PyObject* obj){
  int rv = PyList_Append(list, obj);
  Py_DECREF(obj);
  return rv;
}

static int list_append_string(PyObject* list, const char* val){
  PyObject* obj = PyUnicode_FromString(val);
  return list_append(list, obj);
}

static PyObject* Majka_tags(const char * tag_string) {
  PyObject* tags = PyDict_New();
  char category = ' ';
  char tmp[] = {'\0', '\0'};
  PyObject* type = PyList_New(0);

  if (*tag_string == 'k') {
    ++tag_string;
    category = *tag_string;
    switch (category) {
      case '1':
        dict_set_string(tags, "pos", "substantive");
        break;
      case '2':
        dict_set_string(tags, "pos", "adjective");
        break;
      case '3':
        dict_set_string(tags, "pos", "pronomina");
        break;
      case '4':
        dict_set_string(tags, "pos", "numeral");
        break;
      case '5':
        dict_set_string(tags, "pos", "verb");
        break;
      case '6':
        dict_set_string(tags, "pos", "adverb");
        break;
      case '7':
        dict_set_string(tags, "pos", "preposition");
        break;
      case '8':
        dict_set_string(tags, "pos", "conjuction");
        break;
      case '9':
        dict_set_string(tags, "pos", "particle");
        break;
      case '0':
        dict_set_string(tags, "pos", "interjection");
        break;
      case 'I':
        dict_set_string(tags, "pos", "punctuation");
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'e') {
    ++tag_string;
    switch (*tag_string) {
      case 'A':
        PyDict_SetItemString(tags, "negation", Py_False);
        break;
      case 'N':
        PyDict_SetItemString(tags, "negation", Py_True);
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'a') {
    ++tag_string;
    switch (*tag_string) {
      case 'P':
        dict_set_string(tags, "aspect", "perfect");
        break;
      case 'I':
        dict_set_string(tags, "aspect", "imperfect");
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'm') {
    ++tag_string;
    switch (*tag_string) {
      case 'F':
        dict_set_string(tags, "mode", "infinitive");
        break;
      case 'I':
        dict_set_string(tags, "mode", "present indicative");
        break;
      case 'R':
        dict_set_string(tags, "mode", "imperative");
        break;
      case 'A':
        dict_set_string(tags, "mode", "active participle");
        break;
      case 'N':
        dict_set_string(tags, "mode", "passive participle");
        break;
      case 'S':
        dict_set_string(tags, "mode", "adverbium participle, present");
        break;
      case 'D':
        dict_set_string(tags, "mode", "adverbium participle, past");
        break;
      case 'B':
        dict_set_string(tags, "mode", "future indicative");
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'p') {
    ++tag_string;
    tmp[0] = *tag_string;
    dict_set_numeric(tags, "person", tmp);
    ++tag_string;
  }

  if (*tag_string == 'g') {
    ++tag_string;
    switch (*tag_string) {
      case 'M':
        dict_set_string(tags, "gender", "masculine");
        PyDict_SetItemString(tags, "animate", Py_True);
        break;
      case 'I':
        dict_set_string(tags, "gender", "masculine");
        PyDict_SetItemString(tags, "animate", Py_False);
        break;
      case 'F':
        dict_set_string(tags, "gender", "feminine");
        break;
      case 'N':
        dict_set_string(tags, "gender", "neuter");
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'n') {
    ++tag_string;
    switch (*tag_string) {
      case 'S':
        PyDict_SetItemString(tags, "singular", Py_True);
        break;
      case 'P':
        PyDict_SetItemString(tags, "plural", Py_True);
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'c') {
    ++tag_string;
    tmp[0] = *tag_string;
    dict_set_numeric(tags, "case", tmp);
    ++tag_string;
  }

  if (*tag_string == 'p') {  // Duplicate
    ++tag_string;
    tmp[0] = *tag_string;
    dict_set_numeric(tags, "person", tmp);
    ++tag_string;
  }

  if (*tag_string == 'd') {
    ++tag_string;
    tmp[0] = *tag_string;
    dict_set_numeric(tags, "degree", tmp);
    ++tag_string;
  }

  if (*tag_string == 'x') {
    ++tag_string;
    switch (category) {
      case '1':
        switch (*tag_string) {
          case 'P':
            list_append_string(type, "half");
            break;
          case 'F':
            list_append_string(type, "family surname");
            break;
        }
        break;
      case '3':
        switch (*tag_string) {
          case 'P':
            list_append_string(type, "personal");
            break;
          case 'O':
            list_append_string(type, "possessive");
            break;
          case 'D':
            list_append_string(type, "demonstrative");
            break;
          case 'T':
            list_append_string(type, "deliminative");
            break;
        }
        break;
      case '4':
        switch (*tag_string) {
          case 'C':
            list_append_string(type, "cardinal");
            break;
          case 'O':
            list_append_string(type, "ordinal");
            break;
          case 'R':
            list_append_string(type, "reproductive");
            break;
        }
        break;
      case '6':
        switch (*tag_string) {
          case 'D':
            list_append_string(type, "demonstrative");
            break;
          case 'T':
            list_append_string(type, "delimitative");
            break;
        }
        break;
      case '8':
        switch (*tag_string) {
          case 'C':
            list_append_string(type, "coordinate");
            break;
          case 'S':
            list_append_string(type, "subordinate");
            break;
        }
        break;
      case 'I':
        switch (*tag_string) {
          case '.':
            list_append_string(type, "stop");
            break;
          case ',':
            list_append_string(type, "semi-stop");
            break;
          case '"':
            list_append_string(type, "parenthesis");
            break;
          case '(':
            list_append_string(type, "opening");
            break;
          case ')':
            list_append_string(type, "closing");
            break;
          case '~':
            list_append_string(type, "other");
            break;
        }
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'y') {
    ++tag_string;
    switch (*tag_string) {
      case 'F':
        list_append_string(type, "reflective");
        break;
      case 'Q':
        list_append_string(type, "interrogative");
        break;
      case 'R':
        list_append_string(type, "relative");
        break;
      case 'N':
        list_append_string(type, "negative");
        break;
      case 'I':
        list_append_string(type, "indeterminate");
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 't') {
    ++tag_string;
    switch (*tag_string) {
        case 'S':
            list_append_string(type, "status");
            break;
        case 'D':
            list_append_string(type, "modal");
            break;
        case 'T':
            list_append_string(type, "time");
            break;
        case 'A':
            list_append_string(type, "respect");
            break;
        case 'C':
            list_append_string(type, "reason");
            break;
        case 'L':
            list_append_string(type, "place");
            break;
        case 'M':
            list_append_string(type, "manner");
            break;
        case 'Q':
            list_append_string(type, "extent");
            break;
    }
    ++tag_string;
  }

  if (*tag_string == 'z') {
    ++tag_string;
    switch (*tag_string) {
      case 'S':
        dict_set_string(tags, "subclass", "-s enclictic");
        break;
      case 'Y':
        dict_set_string(tags, "subclass", "conditional");
        break;
      case 'A':
        dict_set_string(tags, "subclass", "abbreviation");
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'w') {
    ++tag_string;
    switch (*tag_string) {
      case 'B':
        dict_set_string(tags, "style", "poeticism");
        break;
      case 'H':
        dict_set_string(tags, "style", "conversational");
        break;
      case 'N':
        dict_set_string(tags, "style", "dialectal");
        break;
      case 'R':
        dict_set_string(tags, "style", "rare");
        break;
      case 'Z':
        dict_set_string(tags, "style", "obsolete");
        break;
    }
    ++tag_string;
  }

  if (*tag_string == '~') {
    ++tag_string;
    tmp[0] = *tag_string;
    dict_set_numeric(tags, "frequency", tmp);
    ++tag_string;
  }

  if (PyList_Size(type)) {
    dict_set(tags, "type", type);
  } else {
    Py_DECREF(type);
  }

  if (*tag_string) {
    dict_set_string(tags, "other", tag_string);
  }

  return tags;
}

static PyObject* Majka_find(Majka* self, PyObject* args, PyObject* kwds) {
  const char* word = NULL;
  char* results = new char[self->majka->max_results_size];
  const char* entry, * colon, * negative;
  char tmp_lemma[300];
  PyObject* ret = PyList_New(0);
  PyObject* lemma, * tags, * option;
  int rc, i;

  static char* kwlist[] = {const_cast<char*>("word"), NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &word)) {
    delete [] results;
    return NULL;
  }

  rc = self->majka->find(word, results, self->flags);

  if (rc == 0) {
    delete [] results;
    return ret;
  }

  if (self->first_only) rc = 1;

#ifdef PY3K
  negative = PyUnicode_AsUTF8(self->negative);
#else
  negative = PyString_AsString(self->negative);
#endif

  for (entry = results, i=0; i < rc; i++, entry += strlen(entry) + 1) {
    colon = strchr(entry, ':');
    memcpy(tmp_lemma, entry, colon-entry);
    tmp_lemma[colon-entry] = '\0';

    if (self->tags) {
      lemma = PyUnicode_FromString(tmp_lemma);
      tags = Majka_tags(colon+1);
      option = Py_BuildValue("{s:O,s:O}",
                             "lemma", lemma,
                             "tags", tags);
      Py_DECREF(tags);
    } else {
      if (is_negation(colon+1)){
        memmove(tmp_lemma+strlen(negative), tmp_lemma, strlen(tmp_lemma)+1);
        memcpy(tmp_lemma, negative, strlen(negative));
      }
      lemma = PyUnicode_FromString(tmp_lemma);
      option = Py_BuildValue("{s:O}",
                             "lemma", lemma);
    }
    Py_DECREF(lemma);

    if (self->compact_tag) {
      dict_set_string(option, "compact_tag", colon+1);
    }

    list_append(ret, option);
  }
  delete [] results;
  return ret;
}

static PyMethodDef Majka_methods[] = {
  {"find", (PyCFunction)Majka_find, METH_VARARGS | METH_KEYWORDS,
   "Get results for given word."
  },
  {NULL}  /* Sentinel */
};

static PyMemberDef Majka_members[] = {
  {const_cast<char*>("flags"), T_INT, offsetof(Majka, flags), 0,
   const_cast<char*>("Flags to run Majka with.")},
  {const_cast<char*>("tags"), T_BOOL, offsetof(Majka, tags), 0,
   const_cast<char*>("If tags should be extracted and converted.")},
  {const_cast<char*>("compact_tag"), T_BOOL, offsetof(Majka, compact_tag), 0,
   const_cast<char*>("If original compact tag string should be extracted and returned.")},
  {const_cast<char*>("first_only"), T_BOOL, offsetof(Majka, first_only), 0,
   const_cast<char*>("If only first match should be returned.")},
  {const_cast<char*>("negative"), T_OBJECT, offsetof(Majka, negative), 0,
   const_cast<char*>("Negative prefix for languages supporting a negative tag.")},
  {NULL}
};

static PyTypeObject MajkaType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "majka.Majka",             /* tp_name */
  sizeof(Majka),             /* tp_basicsize */
  0,                         /* tp_itemsize */
  (destructor)Majka_dealloc, /* tp_dealloc */
  0,                         /* tp_print */
  0,                         /* tp_getattr */
  0,                         /* tp_setattr */
  0,                         /* tp_reserved */
  0,                         /* tp_repr */
  0,                         /* tp_as_number */
  0,                         /* tp_as_sequence */
  0,                         /* tp_as_mapping */
  0,                         /* tp_hash  */
  0,                         /* tp_call */
  0,                         /* tp_str */
  0,                         /* tp_getattro */
  0,                         /* tp_setattro */
  0,                         /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT |
      Py_TPFLAGS_BASETYPE,   /* tp_flags */
  "Majka object",            /* tp_doc */
  0,                         /* tp_traverse */
  0,                         /* tp_clear */
  0,                         /* tp_richcompare */
  0,                         /* tp_weaklistoffset */
  0,                         /* tp_iter */
  0,                         /* tp_iternext */
  Majka_methods,             /* tp_methods */
  Majka_members,             /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)Majka_init,      /* tp_init */
  0,                         /* tp_alloc */
  Majka_new,                 /* tp_new */
};

#ifdef PY3K
static PyModuleDef majkamodule = {
  PyModuleDef_HEAD_INIT,
  "majka",
  "Majka module.",
  -1,
  NULL, NULL, NULL, NULL, NULL
};

#define init_function PyInit_majka
#define init_return(value) return (value)
#else
#define init_function initmajka
#define init_return(unused) return
#endif

PyMODINIT_FUNC init_function(void) {
  PyObject* m;

  if (PyType_Ready(&MajkaType) < 0)
    init_return(NULL);
#ifdef PY3K
  m = PyModule_Create(&majkamodule);
#else
  m = Py_InitModule3("majka", NULL, NULL);
#endif
  if (m == NULL)
    init_return(NULL);

  Py_INCREF(&MajkaType);
  PyModule_AddObject(m, "Majka",
                     reinterpret_cast<PyObject*>(&MajkaType));
  PyModule_AddObject(m, "ADD_DIACRITICS",
                     PyLong_FromLong(ADD_DIACRITICS));
  PyModule_AddObject(m, "IGNORE_CASE",
                     PyLong_FromLong(IGNORE_CASE));
  PyModule_AddObject(m, "DISALLOW_LOWERCASE",
                     PyLong_FromLong(DISALLOW_LOWERCASE));
  init_return(m);
}

