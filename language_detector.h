
#ifndef LANGUAGE_DETECTOR_H
#define LANGUAGE_DETECTOR_H

typedef enum
{
    LANG_UNKNOWN = 0,
    LANG_C,
    LANG_CPP,
    LANG_JAVA,
    LANG_PYTHON
} Language;

Language detect_language(const char *filename);

const char *language_name(Language language);

void assignment_detect_languages(void);

#endif
