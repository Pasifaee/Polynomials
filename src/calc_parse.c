/** @file
  Implementacja kalkulatora wykonującego działania na wielomianach

  @authors Izabela Ożdżeńska <io417924@students.mimuw.edu.pl>
  @date 2021
*/

#define _GNU_SOURCE

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include <stdio.h>

#include "calc_parse.h"
#include "poly.h"

/**
 * Początkowy rozmiar tablicy, której rozmiar może być w przyszłości
 * zwiększany.
 */
#define INITIAL_SIZE 4

/**
 * Sprawdza czy tekst zaczyna się od zadanego prefiksu.
 * @param input : tekst
 * @param prefix : prefiks
 * @return 1, jeśli tekst zaczyna się od zadanego prefiksu; 0 w przeciwnym
 * wypadku
 */
static bool startsWith(const char *input, const char *prefix) {
    if (strlen(prefix) > strlen(input)) return false;
    for (size_t i = 0; prefix[i]; i++) {
        if (input[i] != prefix[i]) return false;
    }
    return true;
}

/**
 * Sprawdza czy zadany tekst można zinterpretować jako wielomian będący
 * współczynnikiem.
 * @param[in] input : tekst
 * @return 1, jeśli zadany tekst można zinterpretować jako współczynnik
 * wielomianu; 0 w przeciwnym wypadku
 */
bool correctCoeff(const char *input) {
    if (input[0] == '\0')return false;
    if (input[0] == '+' || isspace(input[0])) return false;
    char *endptr;
    errno = 0;
    strtol(input, &endptr, 10);
    if (endptr[0] != '\0' || errno) return false;
    else return true;
}

/**
 * Tekst można zinterpretować jako wielomian będący współczynnikiem wtedy
 * i tylko wtedy, gdy można go zinterpretować jako argument polecenia z opcją
 * <AT>, co jest sprawdzane przez funkcję correctCoeff().
 */
#define correctAtArg correctCoeff

/**
 * Sprawdza czy zadany tekst można zinterpretować jako wykładnik potęgi
 * zmiennej jednomianu.
 * @param[in] input : tekst
 * @return 1, jeśli zadany tekst można zinterpretować jako wykładnik potęgi
 * zmiennej jednomianu; 0 w przeciwnym wypadku
 */
bool correctExp(const char *input) {
    if (!correctCoeff(input)) return false;
    else {
        char *endptr;
        long num = strtol(input, &endptr, 10);
        if (num < 0 || num > INT_MAX) return false;
        else return true;
    }
}

/**
 * Sprawdza czy zadany tekst można zinterpretować jako argument polecenia
 * z opcją <DEG_BY>.
 * @param[in] input : tekst
 * @return 1, jeśli zadany tekst można zinterpretować jako argument polecenia
 * z opcją <DEG_BY>; 0 w przeciwnym wypadku
 */
static bool correctDegArg(const char *input) {
    if (input[0] == '\0') return false;
    if (input[0] == '+' || input[0] == '-' || isspace(input[0])) return false;
    char *endptr;
    errno = 0;
    strtoul(input, &endptr, 10);
    if (endptr[0] != '\0' || errno) return false;
    else return true;
}

/**
 * Tekst można zinterpretować jako arguemtn polecenia z opcją <COMPOSE> wtedy
 * i tylko wtedy, gdy można go zinterpretować jako argument polecenia z opcją
 * <DEG_BY>, co jest sprawdzane przez funkcję correctDegArg().
 */
#define correctComposeArg correctDegArg

static bool correctMono(const char *input);

/**
 * Sprawdza czy zadany tekst można zinterpretować jako wielomian. Akceptowane
 * są następujące formaty tekstowe wielomianu:
 * "<współczynnik>", gdzie współczynnik to tekst, który można zinterpretować
 * jako wielomian będący współczynnikiem (ptarz: correctCoeff())
 * "(<jednomian>)"
 * "(<jednomian>)+@f$\ldots@f$+(<jednomian>)", gdzie <jednomian> to tekst,
 * który można zinterpretować jako jednomian (patrz: correctMono())
 * @param[in] input
 * @return 1, jeśli zadany tekst można zinterpretować jako wielomian;
 * 0 w przeciwnym wypadku
 */
bool correctPoly(const char *input) {
    if (input[0] == '\0') return false;
    if (isdigit(input[0]) || input[0] == '-') {
        return correctCoeff(input);
    }
    else {
        size_t idx = 0, start_idx, parentheses;
        while (true) {
            if (input[idx] == '\0') return false;
            start_idx = idx;
            if (input[start_idx] != '(') return false;
            else parentheses = 1;
            // Szukamy nawiasu zamykającego nawias na pozycji [start_idx]
            while (parentheses > 0) {
                idx++;
                if (input[idx] == '\0') return false;
                if (input[idx] == '(') parentheses++;
                if (input[idx] == ')') parentheses--;
            }
            if (idx - start_idx == 1) return false; // Znaleziono "()".
            char *mono = malloc(idx - start_idx);
            mono[idx - start_idx - 1] = '\0'; // To jest tekst pomiędzy
            // nawiasami na pozycjach [start_idx] i [idx].
            strncpy(mono, input + start_idx + 1, idx - start_idx - 1);
            if (!correctMono(mono)) {
                free(mono);
                return false;
            }
            free(mono);

            if (input[idx + 1] == '\0') return true; // [input] zakończył się
            // w odpowiednim momencie.
            else if (input[idx + 1] != '+') return false; // Po zakończonym
            // jednomianie nie wystąpił '+'.
            else idx += 2;
        }
    }
}

/**
 * Sprawdza czy zadany tekst można zinterpretować jako jednomian. Akceptowany
 * jest następujący format tekstowy jednomianu:
 * "<wielomian>,<wykładnik potęgi>", gdzie <wielomian> to tekst, który można
 * zinterpretować jako wielomian (patrz: correctPoly()), a <wykładnik potęgi>
 * to tekst, który można zinterpretować jako wykładnik potęgi (patrz:
 * correctExp()).
 * @param[in] input : tekst
 * @return 1, jeśli zadany tekst można zinterpretować jako jednomian;
 * 0 w przeciwnym wypadku
 */
static bool correctMono(const char *input) {
    if (input[0] == '\0') return false;
    size_t input_len = strlen(input);
    size_t comma_idx = 0; // To jest pozycja, na której znajduje się ostatni
    // przecinek.
    for (size_t i = input_len; i > 0; i--) {
        if (input[i] == ',') {
            comma_idx = i;
            break;
        }
    }
    if (comma_idx == 0 || comma_idx == input_len - 1) return false;
    // Podzielenie [input] na lewą część ([poly]) i prawą część ([exp]).
    char *poly = malloc(comma_idx + 1);
    char *exp = malloc(input_len - comma_idx);
    poly[comma_idx] = '\0', exp[input_len - comma_idx - 1] = '\0';
    strncpy(poly, input, comma_idx);
    strncpy(exp, input + comma_idx + 1, input_len - comma_idx - 1);

    if (correctPoly(poly) && correctExp(exp)) {
        free(poly);
        free(exp);
        return true;
    }
    else {
        free(poly);
        free(exp);
        return false;
    }
}

static Mono StrToMono(const char *input);

/**
 * Konwertuje zadany tekst na wielomian niebędący współczynnikiem.
 * @param[in] input : tekst
 * @return skonwertowany wielomian niebędący współczynnikiem
 */
static Poly StrToPolyNonCoeff(const char *input) {
    Mono *monos = malloc(INITIAL_SIZE * sizeof(Mono)); // To jest tablica
    // jednomianów zawartych w [input].
    size_t monos_idx = 0, monos_size = INITIAL_SIZE;
    size_t idx = 0, start_idx, parentheses;
    while (true) {
        if (monos_idx == monos_size) { // Zwiększ [monos], jeśli jest zapełniona.
            monos_size *= 2;
            monos = realloc(monos, monos_size * sizeof(Mono));
        }
        start_idx = idx;
        parentheses = 1;
        // Szukamy nawiasu zamykającego nawias na pozycji [start_idx]
        while (parentheses > 0) {
            idx++;
            if (input[idx] == '(') parentheses++;
            if (input[idx] == ')') parentheses--;
        }
        char *mono = malloc(idx - start_idx);
        mono[idx - start_idx - 1] = '\0';
        strncpy(mono, input + start_idx + 1, idx - start_idx - 1);

        Mono new_mono = StrToMono(mono);
        monos[monos_idx] = new_mono;
        monos_idx++;
        free(mono);

        if (input[idx + 1] == '\0') break;
        else idx += 2;
    }
    monos_size = monos_idx;
    Poly res = PolyAddMonos(monos_size, monos);
    free(monos);
    return res;
}

/**
 * Konwertuje zadany tekst na wielomian. W przypadku, gdy zadany tekst nie może
 * być zinterpretowany jako wielomian, program kończy działanie.
 * @param[in] input : tekst
 * @return wielomian - wynik konwersji
 */
Poly StrToPoly(const char *input) {
    assert(correctPoly(input));
    if (isdigit(input[0]) || input[0] == '-') {
        char *endptr;
        long coeff = strtol(input, &endptr, 10);
        return PolyFromCoeff(coeff);
    }
    else {
        return StrToPolyNonCoeff(input);
    }
}

/**
 * Konwertuje zadany tekst na jednomian.
 * @param[in] input : tekst
 * @return jednomian - wynik konwersji
 */
static Mono StrToMono(const char *input) {
    assert(input[0] != '\0');
    size_t input_len = strlen(input);
    size_t comma_idx = 0;
    for (size_t i = input_len; i > 0; i--) {
        if (input[i] == ',') {
            comma_idx = i;
            break;
        }
    }
    assert(comma_idx != 0 && comma_idx != input_len - 1);
    // Podzielenie [input] na lewą część (poly) i prawą część (exp).
    char *poly = malloc(comma_idx + 1), *exp = malloc(input_len - comma_idx);
    poly[comma_idx] = '\0', exp[input_len - comma_idx - 1] = '\0';
    strncpy(poly, input, comma_idx);
    strncpy(exp, input + comma_idx + 1, input_len - comma_idx - 1);

    char *endptr;
    Poly p = StrToPoly(poly);
    poly_exp_t e = (int) strtol(exp, &endptr, 10);
    free(poly);
    free(exp);
    if (PolyIsZero(&p) && e != 0) return MonoFromPoly(&p, 0);
    else return MonoFromPoly(&p, e);
}

/**
 * To jest struktura przechowująca tekst razem z jego długością.
 */
typedef struct StringPair {
    char *string;   ///< tekst
    size_t size;    ///< długość tekstu
} StringPair;

static StringPair MonoToStrPair(const Mono *m);

/**
 * Konwertuje wielomian na tekst w formie struktury StringPair, w formacie
 * opisanym w dokumentacji funkcji correctPoly().
 * @param[in] p : wielomian
 * @return tekst będący wynikiem konwersji
 */
static StringPair PolyToStrPair(const Poly *p) {
    char *res = NULL;
    size_t size = 0;
    if (PolyIsCoeff(p)) {
        if (!asprintf(&res, "%ld", p->coeff)) exit(1);
        size = strlen(res);
    }
    else {
        StringPair monos_str[p->size];
        for (size_t i = 0; i < p->size; i++) {
            monos_str[i] = MonoToStrPair(&p->arr[p->size - i - 1]);
            size += monos_str[i].size;
        }
        size_t parentheses = 3 * p->size - 1;
        size += parentheses;
        res = realloc(res, size + 1);

        size_t res_idx = 0;
        for (size_t i = 0; i < p->size; i++) {
            res[res_idx] = '(';
            res_idx++;
            strcpy(res + res_idx, monos_str[i].string);
            res_idx += monos_str[i].size;
            res[res_idx] = ')';
            res_idx++;
            if (res_idx == size) res[res_idx] = '\0';
            else res[res_idx] = '+';
            res_idx++;

            free(monos_str[i].string);
        }
    }
    return (StringPair) {.string = res, .size = size};
}

/**
 * Konwertuje jednomian na tekst w formie struktury StringPair, w formacie
 * opisanym w dokumentacji funkcji correctMono().
 * @param[in] m : jednomian
 * @return tekst będący wynikiem konwersji
 */
static StringPair MonoToStrPair(const Mono *m) {
    char *res = NULL;
    size_t size;

    StringPair poly = PolyToStrPair(&m->p);
    char *exp;
    if (!asprintf(&exp, "%d", m->exp)) exit(1);
    size_t exp_size = strlen(exp);

    size = poly.size + exp_size + 1;
    res = malloc(size + 1);
    strcpy(res, poly.string);
    res[poly.size] = ',';
    strcpy(res + poly.size + 1, exp);
    free(exp);
    res[size] = '\0';

    free(poly.string);

    return (StringPair) {.string = res, .size = size};
}

/**
 * Konwertuje wielomian na tekst w formacie opisanym w dokumentacji funkcji
 * correctPoly().
 * @param[in] p : wielomian
 * @return tekst będący wynikiem konwersji
 */
char* PolyToStr(const Poly *p) {
    return PolyToStrPair(p).string;
}

/**
 * Sprawdza czy na stosie jest co najmniej @p n wielomianów. Jeśli tak,
 * zwraca polecenie z zadaną opcją @p option; jeśli nie zwraca polecenie
 * z opcją <error>.
 * @param[in] stack : stos wielomianów
 * @param[in] n : liczba wielomianów
 * @param[in] option : opcja polecenia
 * @param[in] verse_num : numer wiersza standardowego wejścia, w którym zostało
 * podane polecenie
 * @return jeśli na stosie jest co najmniej @p n wielomianów - polecenie
 * z zadaną opcją @p option; w przeciwnym wypadku - polecenie z opcją <error>
 */
static Command CheckUnderflow(Stack stack, size_t n, Option option,
                              size_t verse_num) {
    if (hasnElements(stack, n)) {
        return (Command) {.opt = option};
    }
    else {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", verse_num);
        return (Command) {.opt = error};
    }
}

/**
 * Sprawdza czy wystąpiły błędy przy identyfikacji polecenia "DEG_BY". Jeśli
 * wystąpiły błędy, wypisuje na standardowe wyjście diagnostyczne komunikat
 * o błędzie i zwraca polecenie z opcją <error>. Jeśli nie wystąpiły błędy,
 * zwraca polecenie z opcją <DEG_BY> i argumentem podanym w @p input.
 * Możliwe błędy to nieprawidłowy argument i zbyt mało wielomianów na stosie.
 * @param[in] stack : stos wielomianów
 * @param[in] input : tekst polecenia
 * @param[in] verse_num : numer wiersza standardowego wejścia, w którym zostało
 * podane polecenie
 * @return jeśli wystąpiły błędy - polecenie z opcją <error>; w przeciwnym
 * wypadku - polecenie z opcją <DEG_BY> i argumentem podanym w @p input
 */
static Command CheckDegErr(Stack stack, const char *input, size_t verse_num) {
    size_t deg_len = 7;
    const char *arg = input + deg_len;
    if (correctDegArg(arg)) {
        Command res = CheckUnderflow(stack, 1, DEG_BY, verse_num);
        if (res.opt != error) {
            char *endptr;
            unsigned long deg_arg = strtoul(arg, &endptr, 10);
            res.deg_arg = deg_arg;
        }
        return res;
    }
    else {
        fprintf(stderr, "ERROR %zu DEG BY WRONG VARIABLE\n", verse_num);
        return (Command) {.opt = error};
    }
}

/**
 * Sprawdza czy wystąpiły błędy przy identyfikacji polecenia "AT". Jeśli
 * wystąpiły błędy, wypisuje na standardowe wyjście diagnostyczne komunikat
 * o błędzie i zwraca polecenie z opcją <error>. Jeśli nie wystąpiły błędy,
 * zwraca polecenie z opcją <AT> i argumentem podanym w @p input.
 * Możliwe błędy to nieprawidłowy argument i zbyt mało wielomianów na stosie.
 * @param[in] stack : stos wielomianów
 * @param[in] input : tekst polecenia
 * @param[in] verse_num : numer wiersza standardowego wejścia, w którym zostało
 * podane polecenie
 * @return jeśli wystąpiły błędy - polecenie z opcją <error>; w przeciwnym
 * wypadku - polecenie z opcją <AT> i argumentem podanym w @p input
 */
static Command CheckAtErr(Stack stack, const char *input, size_t verse_num) {
    size_t at_len = 3;
    char const *arg = input + at_len;
    if (correctAtArg(arg)) {
        Command res = CheckUnderflow(stack, 1, AT, verse_num);
        if (res.opt != error) {
            char *endptr;
            long at_arg = strtol(arg, &endptr, 10);
            res.at_arg = at_arg;
        }
        return res;
    }
    else {
        fprintf(stderr, "ERROR %zu AT WRONG VALUE\n", verse_num);
        return (Command) {.opt = error};
    }
}

/**
 * Sprawdza czy wystąpiły błędy przy identyfikacji polecenia "COMPOSE". Jeśli
 * wystąpiły błędy, wypisuje na standardowe wyjście diagnostyczne komunikat
 * o błędzie i zwraca polecenie z opcją <error>. Jeśli nie wystąpiły błędy,
 * zwraca polecenie z opcją <COMPOSE> i argumentem podanym w @p input.
 * Możliwe błędy to nieprawidłowy argument i zbyt mało wielomianów na stosie.
 * @param[in] stack : stos wielomianów
 * @param[in] input : tekst polecenia
 * @param[in] verse_num : numer wiersza standardowego wejścia, w którym zostało
 * podane polecenie
 * @return jeśli wystąpiły błędy - polecenie z opcją <error>; w przeciwnym
 * wypadku - polecenie z opcją <COMPOSE> i argumentem podanym w @p input
 */
static Command CheckComposeErr(Stack stack, const char *input, size_t verse_num) {
    size_t compose_len = 8;
    char const *arg = input + compose_len;
    if (correctComposeArg(arg)) {
        char *endptr;
        size_t k = strtoul(arg, &endptr, 10);
        Command res = CheckUnderflow(stack, k, COMPOSE, verse_num);
        if (res.opt != error) res.compose_arg = k;
        return res;
    }
    else {
        fprintf(stderr, "ERROR %zu COMPOSE WRONG PARAMETER\n", verse_num);
        return (Command) {.opt = error};
    }
}

/**
 * Sprawdza, czy tekst polecenia reprezentuje jedno ze słownych poleceń
 * z argumentem: "DEG_BY", "AT" lub "COMPOSE" oraz czy nie wystąpił błąd przy
 * ich przetwarzaniu. Jeśli tekst polecenia nie reprezentuje jednego ze słownych
 * poleceń z argumentem lub wystąpił błąd przy przetwarzaniu tych poleceń,
 * wypisuje komunikat o błędzie na standardowe wyjście diagnostyczne i zwraca
 * polecenie z opcją <error>. W przeciwnym wypadku zwraca polecenie z opcją mu
 * odpowiadającą i argumentem podanym w @p input.
 * @param[in] stack : stos wielomianów
 * @param[in] input : tekst polecenia
 * @param[in] verse_num : numer wiersza standardowego wejścia, w którym został
 * podany słowny zapis polecenia (@p input)
 * @return jeśli tekst polecenia nie reprezentuje jednego ze słownych
 * poleceń z argumentem lub wystąpił błąd przy przetwarzaniu tych poleceń -
 * polecenie z opcją <error>; w przeciwnym wypadku polecenie z odpowiednią opcją
 * i argumentem podanym w @p input.
 */
static Command IdentifyArgCommand(Stack stack, const char *input, size_t verse_num) {
    if (startsWith(input, "DEG_BY ")) {
        return CheckDegErr(stack, input, verse_num);
    }
    else if (startsWith(input, "AT ")) {
        return CheckAtErr(stack, input, verse_num);
    }
    else if (startsWith(input, "COMPOSE ")) {
        return CheckComposeErr(stack, input, verse_num);
    }
    else {
        if (startsWith(input, "DEG_BY")) {
            fprintf(stderr, "ERROR %zu DEG BY WRONG VARIABLE\n",
                    verse_num);
        }
        else if (startsWith(input, "AT")) {
            fprintf(stderr, "ERROR %zu AT WRONG VALUE\n", verse_num);
        }
        else if (startsWith(input, "COMPOSE")) {
            fprintf(stderr, "ERROR %zu COMPOSE WRONG PARAMETER\n",
                    verse_num);
        }
        else {
            fprintf(stderr, "ERROR %zu WRONG COMMAND\n", verse_num);
        }

        return (Command) {.opt = error};
    }
}

/**
 * Przyjmuje tekst polecenia podany przez użytkownika i zwraca polecenie,
 * które on reprezentuje. W przypadku nieprawidłowej nazwy polecenia,
 * nieprawdiłowego argumentu lub zbyt małej liczby wielomianów na stosie,
 * wypisuje informację o błędzie na standardowe wyjście diagnostyczne i zwraca
 * polecenie z opcją <error>.
 * @param[in] stack : stos wielomianów
 * @param[in] input : tekst polecenia
 * @param[in] verse_num : numer wiersza standardowego wejścia, w którym został
 * podany słowny zapis polecenia (@p input)
 * @return polecenie reprezentowane przez @p input lub polecenie z opcją
 * <error> w przypadku błędu
 */
Command IdentifyCommand(Stack stack, const char *input, size_t verse_num) {
    assert(input && input[0] != '#' && input[strlen(input) - 1] != '\n');
    if (isalpha(input[0])) { // Przetwarzanie słownego polecenia.
        if (strcmp(input, "ZERO") == 0) return (Command) {.opt = ZERO};
        else if (strcmp(input, "IS_COEFF") == 0) return CheckUnderflow(stack, 1, IS_COEFF, verse_num);
        else if (strcmp(input, "IS_ZERO") == 0) return CheckUnderflow(stack, 1, IS_ZERO, verse_num);
        else if (strcmp(input, "CLONE") == 0) return CheckUnderflow(stack, 1, CLONE, verse_num);
        else if (strcmp(input, "ADD") == 0) return CheckUnderflow(stack, 2, ADD, verse_num);
        else if (strcmp(input, "MUL") == 0) return CheckUnderflow(stack, 2, MUL, verse_num);
        else if (strcmp(input, "NEG") == 0) return CheckUnderflow(stack, 1, NEG, verse_num);
        else if (strcmp(input, "SUB") == 0) return CheckUnderflow(stack, 2, SUB, verse_num);
        else if (strcmp(input, "IS_EQ") == 0) return CheckUnderflow(stack, 2, IS_EQ, verse_num);
        else if (strcmp(input, "DEG") == 0) return CheckUnderflow(stack, 1, DEG, verse_num);
        else if (strcmp(input, "PRINT") == 0) return CheckUnderflow(stack, 1, PRINT, verse_num);
        else if (strcmp(input, "POP") == 0) return CheckUnderflow(stack, 1, POP, verse_num);
        else return IdentifyArgCommand(stack, input, verse_num);
    }
    else {
        if (correctPoly(input)) { // Polecenie dodania wielomianu.
            Poly p = StrToPoly(input);
            return (Command) {.opt = add_poly, .p = p};
        }
        else {
            fprintf(stderr, "ERROR %zu WRONG POLY\n", verse_num);
            return (Command) {.opt = error};
        }
    }
}

/**
 * Wykonuje polecenie "COMPOSE".
 * @param[in,out] stack : stos
 * @param[in] command : polecenie
 */
void Compose(Stack *stack, Command command) {
    Poly p, q[command.compose_arg];
    p = pop(stack);
    for (size_t i = command.compose_arg; i-- > 0;) {
        q[i] = pop(stack);
    }
    push(stack, PolyCompose(&p, command.compose_arg, q));

    PolyDestroy(&p);
    for (size_t i = 0; i < command.compose_arg; i++) {
        PolyDestroy(&q[i]);
    }
}

/**
 * Wykonuje zadane polecenie wykonując operacje na stosie wielomianów i/lub
 * wypisując wynik operacji na standardowe wyjście.
 * @param[in,out] stack : stos wielomianów
 * @param[in] command : polecenie
 */
void Execute(Stack *stack, Command command) {
    Poly top, top1, top2;
    switch (command.opt) {
        case ZERO: ;
            Poly p = PolyZero();
            push(stack, p);
            break;
        case IS_COEFF: ;
            top = nthElement(*stack, 0);
            printf("%d\n", PolyIsCoeff(&top));
            break;
        case IS_ZERO: ;
            top = nthElement(*stack, 0);
            printf("%d\n", PolyIsZero(&top));
            break;
        case CLONE: ;
            top = nthElement(*stack, 0);
            Poly clone = PolyClone(&top);
            push(stack, clone);
            break;
        case ADD:   ;
            top1 = pop(stack), top2 = pop(stack);
            Poly add = PolyAdd(&top1, &top2);
            push(stack, add);
            PolyDestroy(&top1);
            PolyDestroy(&top2);
            break;
        case MUL: ;
            top1 = pop(stack), top2 = pop(stack);
            Poly mul = PolyMul(&top1, &top2);
            push(stack, mul);
            PolyDestroy(&top1);
            PolyDestroy(&top2);
            break;
        case NEG: ;
            top = pop(stack);
            Poly neg = PolyNeg(&top);
            push(stack, neg);
            PolyDestroy(&top);
            break;
        case SUB: ;
            top1 = pop(stack), top2 = pop(stack);
            Poly sub = PolySub(&top1, &top2);
            push(stack, sub);
            PolyDestroy(&top1);
            PolyDestroy(&top2);
            break;
        case IS_EQ: ;
            top1 = nthElement(*stack, 0), top2 = nthElement(*stack, 1);
            printf("%d\n", PolyIsEq(&top1, &top2));
            break;
        case DEG: ;
            top = nthElement(*stack, 0);
            printf("%d\n", PolyDeg(&top));
            break;
        case PRINT: ;
            top = nthElement(*stack, 0);
            char *poly = PolyToStr(&top);
            printf("%s\n", poly);
            free(poly);
            break;
        case POP: ;
            top = pop(stack);
            PolyDestroy(&top);
            break;
        case DEG_BY: ;
            top = nthElement(*stack, 0);
            printf("%d\n", PolyDegBy(&top, command.deg_arg));
            break;
        case AT: ;
            top = pop(stack);
            Poly at = PolyAt(&top, command.at_arg);
            push(stack, at);
            PolyDestroy(&top);
            break;
        case COMPOSE:
            Compose(stack, command);
            break;
        case add_poly:
            push(stack, command.p);
            break;
        case error:
            break;
    }
}

/**
 * Wczytuje kolejne wiersze ze standardowego wejścia, sprawdza jakie polecenie
 * jest zawarte w każdym wierszu, a następnie wykonuje to polecenie, wykonując
 * operacje na stosie wielomianów i/lub wypisując wynik operacji na standardowe
 * wyjście. Stos wielomianów jest pusty na początku programu i jest modyfikowany
 * za pomocą podanych poleceń. W przypadku podania przez użytkownika
 * nieprawidłowej nazwy polecenia, nieprawidłowego argumentu lub gdy polecenie
 * nie może zostać wykonane ze względu na zbyt małą liczbę wielomianów na stosie,
 * na standardowe wyjście diagnostyczne wypisywany jest komunikat o błędzie.
 */
void GetInput(void) {
    Stack stack = create();
    char *input = NULL;
    size_t verse_num = 1, getline_size = 0;
    long getline_out = getline(&input, &getline_size, stdin);
    // Wczytywanie wierszy.
    while (getline_out != -1) {
        // Puste wiersze i wiersze zaczynające się od '#' są ignorowane.
        if (input[0] != '#' && input[0] != '\n') {
            // Zmieniamy ostatni znak z '\n' na '\0', żeby uprościć [input].
            if (input[getline_out - 1] == '\n') input[getline_out - 1] = '\0';
            // Wykonujemy polecenie.
            Command command = IdentifyCommand(stack, input, verse_num);
            if (command.opt != error) {
                Execute(&stack, command);
            }
        }
        getline_out = getline(&input, &getline_size, stdin);
        verse_num++;
    }
    free(input);
    // Usuwamy ze stosu wielomiany, które zostały.
    while (hasnElements(stack, 1)) {
        Poly top = pop(&stack);
        PolyDestroy(&top);
    }
}