/* test_framework.h - Mini framework di test senza dipendenze */
#ifndef ECLYRA_TEST_FRAMEWORK_H
#define ECLYRA_TEST_FRAMEWORK_H
#include <stdio.h>

static int test_eseguiti = 0, test_falliti = 0;

#define VERIFICA(cond) do { test_eseguiti++; if (!(cond)) { test_falliti++; \
    printf("  FALLITO  %s:%d  %s\n", __FILE__, __LINE__, #cond); } } while (0)

#define FINE_TEST() (printf("%d verifiche, %d fallite\n", test_eseguiti, test_falliti), test_falliti ? 1 : 0)

#endif
