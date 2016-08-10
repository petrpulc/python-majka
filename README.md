#Python wrapper for Majka

##What is Majka
Majka is a linguistics tool for morphology analysis.

Depending on passed dictionaries, the searched word form is translated to a lemma (basic form) and tags determining the linguistic properties of the word or other way around.

##For what can be this module used?
The lemmatization of documents is heavily used in text processing, as it simplifies the processing of text in inflective languages.

An example from Czech: 'dělala' (she did) is transformed to 'dělat' (do) and tags determining the past tense, female gender and others are added.

Tags returned from the analyzer that comply with new tagset reference (for example cs, sk) are transcribed into a native Python dictionary to enable a much more Python-like experience without a need to study the documentation. Other or falsely recognized are stored in entry 'other'.

##Install / Build instructions
Module is available in PyPi, use `pip install majka` to install.

For local build / install use:

    ./setup.py build
    ./setup.py install

No dependencies outside standard Python and C++ build environment should be needed. (gcc, python-dev, etc.)

##Usage
    import majka
    morph = majka.Majka('path/to/dict')

    morph.flags |= majka.ADD_DIACRITICS  # find word forms with diacritics
    morph.flags |= majka.DISALLOW_LOWERCASE  # do not enable to find lowercase variants
    morph.flags |= majka.IGNORE_CASE  # ignore the word case whatsoever
    morph.flags |= 0  # unset all flags

    morph.tags = False  # return just the lemma, do not process the tags
    morph.tags = True  # turn tag processing back on

    morph.find('nejvhodnější')

##Returns
    [{'lemma': 'vhodný',
      'tags': {'case': 1,
               'gender': 'feminine',
               'negation': False,
               'plural': True,
               'pos': 'adjective',
               'degree': 3}
     },
    ...
    ]

##Attributions
The module is based on code of Pavel Smerk and Pavel Rychly, NLP group at MUNI, Czech Republic.

Original `majka` binary is in `majka/majka_bin.cc`, see `majka/Makefile` for build.

##Additional required downloads
Morphological automatons are distributed separately under different licenses.
See http://nlp.fi.muni.cz/ma/
