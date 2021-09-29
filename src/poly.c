/** @file
  Implementacja klasy wielomianów rzadkich wielu zmiennych

  @authors Izabela Ożdżeńska <io417924@students.mimuw.edu.pl>
  @date 2021
*/

#include <stdlib.h>
#include "poly.h"

/**
 * Podnosi liczbę rzeczywistą do potęgi naturalnej. W przypadku, gdy
 * @f$exp < 0@f$, program kończy działanie.
 * @param[in] x : podstawa potęgi @f$x@f$
 * @param[in] exp : wykładnik @f$exp@f$
 * @return @f$x^{exp}@f$
 */
static poly_coeff_t power(poly_coeff_t x, poly_exp_t exp) {
    assert(exp >= 0);
    if (exp == 0) return 1;
    if (exp == 1) return x;

    poly_coeff_t y = power(x, exp / 2);
    y = y * y;

    if (exp % 2 == 0) return y;
    else return x * y;
}

/**
 * Usuwa wielomian z pamięci.
 * @param[in] p : wielomian
 */
void PolyDestroy(Poly *p) {
    assert(p != NULL);
    // Nie było zaalokowanej pamięci, więc nie ma nic do zwolnienia.
    if (PolyIsCoeff(p)) return;
    for (size_t i = 0; i < p->size; i++) {
        MonoDestroy(&p->arr[i]);
    }
    free(p->arr);
}

/**
 * Robi pełną, głęboką kopię wielomianu.
 * @param[in] p : wielomian
 * @return skopiowany wielomian
 */
Poly PolyClone(const Poly *p) {
    assert(p != NULL);
    if (PolyIsCoeff(p)) return *p;

    Poly q = (Poly) {.size = p->size};
    q.arr = malloc(p->size * sizeof(Mono));
    if (q.arr == NULL) exit(1); // Błąd podczas alokacji pamięci.
    for (size_t i = 0; i < p->size; i++) {
        q.arr[i] = MonoClone(&p->arr[i]);
    }
    return q;
}

/**
 * Z wielomianu @p p będącego współczynnikiem tworzy wielomian złożony z
 * pojedynczego jednomianu o postaci @f$p*x_0^0@f$.
 * @param[in] p : wielomian @f$p@f$ będący współczynnikiem
 * @return @f$p*x_0^0@f$
 */
static Poly ExtendPoly(const Poly *p) {
    assert(p != NULL);
    assert(PolyIsCoeff(p));
    assert(!PolyIsZero(p));
    Poly p_mod = (Poly) {.size = 1, .arr = malloc(sizeof(Mono))};
    if (p_mod.arr == NULL) exit(1); // Błąd podczas alokacji pamięci.
    p_mod.arr[0] = MonoFromPoly(p, 0);
    return p_mod;
}

/**
 * Ustawia wartości @p mono i @p exp na wartość jednomianu i wykładnika
 * jednomianu na pozycji @p idx tablicy jednomianów należącej do @p p.
 * Jeśli @p idx wykracza poza wielkość tablicy jednomianów, ustawia @p exp na -1,
 * a @p mono na NULL.
 * @param[out] mono : jednomian na pozycji @p idx tablicy jednomianów należącej
 * do @p p
 * @param[out] exp : wykładnik jednomianu na pozycji @p idx tablicy jednomianów
 * należącej do @p p
 * @param[in] p : wielomian
 * @param[in] idx : indeks @f$idx@f$
 */
static void SetExpAndMono(Mono *mono, poly_exp_t *exp, const Poly *p, size_t idx) {
    assert(p != NULL);
    if (idx >= p->size) {
        mono = NULL;
        *exp = -1;
    }
    else {
        *mono = p->arr[idx];
        *exp = MonoGetExp(mono);
    }
}

/**
 * Ustawia parametry wielomianu na @p arr i @p size.
 * @param[in] arr : lista jednomianów
 * @param[in] size : liczba jednomianów
 * @return wielomian z parametrami @p arr i @p size
 */
static Poly PolyFromArr(Mono arr[], size_t size) {
    assert(arr != NULL);
    return (Poly) {.size = size, .arr = arr};
}

/**
 * Tworzy wielomian z listy jednomianów. Jeśli suma jednomianów z @p arr
 * redukuje się do wielomianu będącego współczynnikiem, zwraca ów współczynnik.
 * W przeciwnym wypadku zwraca wielomian z zadanymi parametrami @p arr i @p size.
 * @param[in] arr : lista jednomianów
 * @param[in] size : liczba jednomianów
 * @return jeśli suma jednomianów z @p arr redukuje się do współczynnika
 * @f$c@f$ - @f$c@f$; w przeciwnym wypadku wielomian z zadanymi parametrami
 * @p arr i @p size
 */
static Poly PolyFromArrSimplify(Mono arr[], size_t size) {
    assert(arr != NULL);
    assert(!(size == 1 && PolyIsZero(&arr[0].p)));
    Poly res;
    if (size == 0) {
        res = PolyZero();
        free(arr);
    }
    else if (size == 1 && arr[0].exp == 0 && PolyIsCoeff(&arr[0].p)) {
        res = arr[0].p;
        free(arr);
    }
    else {
        res = PolyFromArr(arr, size);
    }
    return res;
}

/**
 * Zwraca sumę jednomianów @p m1 i @p m2, jeśli mają te same wykładniki lub
 * jednomian o większym wykładniku w przeciwnym wypadku. Przesuwa indeksy @p idx1
 * i @p idx2 odpowiednio, aby wskazywały kolejne jednomiany do zsumowania.
 *
 * Zakłada, że @p m1 i @p m2 są jednomianami należącymi do list jednomianów
 * pewnych wielomianów (nazwijmy je @p arr1 i @p arr2) oraz że są one odpowiednio
 * na pozycjach @p idx1 i @p idx2 tych list. Zakłada, że listy te są posortowane
 * malejąco względem wykładników.
 *
 * @param[in] m1 : jednomian @p m1
 * @param[in] exp1 : wykładnik @f$exp_1@f$ jednomianu @p m1 lub @f$-1@f$, jeśli
 * @p idx1 wykracza poza wielkość listy @p arr1
 * @param[in] m2 : jednomian @p m2
 * @param[in] exp2 : wykładnik @f$exp_2@f$ jednomianu @p m2 lub @f$-1@f$, jeśli
 * @p idx2 wykracza poza wielkość listy @p arr2
 * @param[out] idx1 : indeks listy @p arr1
 * @param[out] idx2 : indeks listy @p arr2
 * @return suma jednomianów @p m1 i @p m2, jeśli @f$exp_1 = exp_2@f$ lub
 * jednomian o większym wykładniku w przeciwnym przypadku
 */
static Mono MonoAdd(const Mono *m1, poly_exp_t exp1, const Mono *m2,
                    poly_exp_t exp2, size_t *idx1, size_t *idx2) {
    Mono res;
    // Jeśli wykładniki są równe, dodajemy do siebie wielomiany tych jednomianów.
    if (exp1 == exp2) {
        Poly poly_add = PolyAdd(&(m1->p), &(m2->p));
        if (PolyIsZero(&poly_add)) {
            res = MonoFromPoly(&poly_add, 0);
        }
        else {
            res = MonoFromPoly(&poly_add, exp1);
        }
        (*idx1)++;
        (*idx2)++;
    }
    // Jeśli [exp1] > [exp2], zwiększ [idx1], bo tablice jednomianów
    // są posortowane malejąco względem wykładników.
    else if (exp1 > exp2) {
        res = MonoClone(m1);
        (*idx1)++;
    }
    else {
        res = MonoClone(m2);
        (*idx2)++;
    }
    return res;
}

/**
 * Dodaje dwa wielomiany niebędące współczynnikami.
 * @param[in] p : wielomian @f$p@f$ niebędący współczynnikiem
 * @param[in] q : wielomian @f$q@f$ niebędący współczynnikiem
 * @return @f$p + q@f$
 */
static Poly PolyAddNonCoeffs(const Poly *p, const Poly *q) {
    assert(!PolyIsCoeff(p) && !PolyIsCoeff(q));
    /**
     * To jest tablica, która będzie przechowywała zsumowane jednomiany.
     */
    Mono *arr = malloc((p->size + q->size) * sizeof(Mono));
    if (arr == NULL) exit(1); // Błąd podczas alokacji pamięci.
    size_t arr_size = 0, p_idx = 0, q_idx = 0;
    poly_exp_t p_exp, q_exp;
    Mono p_mono, q_mono;
    // Iterujemy się po kolejnych jednomianach z [p->arr] i [q->arr].
    while (p_idx < p->size || q_idx < q->size) {
        SetExpAndMono(&p_mono, &p_exp, p, p_idx);
        SetExpAndMono(&q_mono, &q_exp, q, q_idx);
        Mono add = MonoAdd(&p_mono, p_exp, &q_mono, q_exp, &p_idx, &q_idx);
        // Jeśli [add] jest tożsamościowo równy 0, nie wstawiamy go do [arr]
        if (!PolyIsZero(&add.p)) {
            arr[arr_size] = add;
            arr_size++;
        }
    }

    if (arr_size != 0) arr = realloc(arr, arr_size * sizeof(Mono));
    if (arr == NULL) exit(1); // Błąd podczas alokacji pamięci.
    Poly res = PolyFromArrSimplify(arr, arr_size);
    return res;
}

/**
 * Dodaje dwa wielomiany.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p + q@f$
 */
Poly PolyAdd(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p->coeff + q->coeff);
    }
    else if (PolyIsCoeff(p)) {
        if (PolyIsZero(p)) return PolyClone(q);
        Poly p_extended = ExtendPoly(p);
        Poly res = PolyAdd(&p_extended, q);
        PolyDestroy(&p_extended);
        return res;
    }
    else if (PolyIsCoeff(q)) {
        return PolyAdd(q, p);
    }
    else {
        return PolyAddNonCoeffs(p, q);
    }
}

/**
 * Porównuje ze sobą dwa jednomiany.
 * @param[in] fst : jednomian o wykładniku @f$exp_1@f$
 * @param[in] snd : jednomian o wykładniku @f$exp_2@f$
 * @return 0, jeśli @f$exp_1 = exp_2@f$; 1 jeśli @f$exp_1 < exp_2@f$; -1 jeśli
 * @f$exp_1 > exp_2@f$
 */
static int CompareMonos(const void *fst, const void *snd) {
    assert(fst != NULL && snd != NULL);
    Mono m1 = *(Mono*)fst, m2 = *(Mono*)snd;
    if (MonoGetExp(&m1) == MonoGetExp(&m2)) return 0;
    else if (MonoGetExp(&m1) < MonoGetExp(&m2)) return 1;
    else return -1;
}

/**
 * Robi płytką kopię listy jednomianów. Kopiuje zawartość listy @p source do
 * listy @p destination.
 * @param[out] destination : skopiowana lista jednomianów
 * @param[in] source : kopiowana lista jednomianów
 * @param[in] size : liczba kopiowanych jednomianów
 */
static void CopyMonos(Mono destination[], const Mono source[], size_t size) {
    assert(source != NULL);
    for (size_t i = 0; i < size; i++) {
        destination[i] = source[i];
    }
}

/**
 * Usuwa z tablicy jednomianów jednomiany na podanych pozycjach. Jeśli @p clone
 * @f$=@f$ <false>, usuwa te jednomiany z pamięci.
 * @param[in,out] monos : tablica jednomianów
 * @param[in] monos_size : liczba jednomianów w @p monos
 * @param[in] remove : lista indeksów jednomianów, które mają zostać
 * usunięte z @p monos
 * @param[in] remove_size : liczba jednomianów do usunięcia
 * @param[in] clone : określa czy usuwane z @p monos jednomiany mają być
 * zachowane w pamięci - jeśli równy <false> - usuwa te jednomiany z pamięci
 */
static void RemoveMonos(Mono *monos, size_t monos_size, const size_t remove[],
                        size_t remove_size, bool clone) {
    assert(remove_size <= monos_size);
    size_t remove_idx = 0, monos_idx = 0;
    for (size_t i = 0; i < monos_size; i++) {
        if (remove_idx == remove_size || i != remove[remove_idx]) {
            monos[monos_idx] = monos[i];
            monos_idx++;
        }
        else {
            if (!clone) MonoDestroy(&monos[i]);
            remove_idx++;
        }
    }
    assert(monos_idx + remove_idx == monos_size && remove_idx == remove_size);
}

/**
 * Usuwa z tablicy jednomianów wszystkie jednomiany, które są tożsamościowo
 * równe zeru.
 * @param[in,out] monos : tablica jednomianów
 * @param[in] count : wielkość tablicy jednomianów
 * @param[out] new_size : wielkość tablicy jednomianów po usunięciu zer
 */
static void RemoveZeros(Mono *monos, size_t count, size_t *new_size) {
    assert(count != 0);
    size_t remove[count];
    size_t remove_size = 0;
    for (size_t i = 0; i < count; i++) {
        if (PolyIsZero(&monos[i].p)) {
            remove[remove_size] = i;
            remove_size++;
        }
    }
    // Ustawiamy parametr [clone] na true, bo wielomiany tożsamościowo
    // równe zero nie zajmują pamięci na stercie, więc nie ma pamięci
    // do zwolnienia.
    RemoveMonos(monos, count, remove, remove_size, true);
    *new_size = count - remove_size;
}

/**
 * Sumuje jednomiany o tych samych potęgach, aktualizując listę jednomianów.
 * Zaktualizowana lista jednomianów sumuje się do tego samego wielomianu, co
 * początkowa lista jednomianów. Wykładniki jednomianów w zaktualizowanej liście
 * nie powtarzają się. Jeśli @p clone @f$=@f$ <false>, usuwa niepotrzebne
 * jednomiany z pamięci.
 * @param[in,out] monos : lista jednomianów
 * @param[in] count : wielkość listy jednomianów
 * @param[out] new_size : wielkość tablicy jednomianów po złączeniu potęg
 * @param[in] clone : określa czy usuwane z @p monos jednomiany mają być
 * zachowane w pamięci - jeśli równy <false> - usuwa te jednomiany z pamięci
 */
static void JoinExponents(Mono *monos, size_t count, size_t *new_size,
                          bool clone) {
    assert(count != 0);
    size_t remove[count], remove_size = 0;
    size_t last_same = -1; // To jest ostatnia pozycja, na której wykładnik się
    // powtórzył.
    if (clone && count == 1) monos[0] = MonoClone(&monos[0]);
    for (size_t i = 1; i < count; i++) {
        // Równe wykładniki mogą znajdować się tylko na sąsiednich pozycjach,
        // ponieważ tablica [monos] jest posortowana.
        if (monos[i].exp == monos[i - 1].exp) {
            Poly add = PolyAdd(&monos[i].p, &monos[i - 1].p);
            if (!clone || last_same == i - 1) MonoDestroy(&monos[i]);
            last_same = i;
            // Jednomian na większej pozycji staje się sumą dwóch jednomianów.
            monos[i].p = add;
            // Jednomian na mniejszej pozycji zostaje usunięty.
            remove[remove_size] = i - 1;
            remove_size++;
        }
        else if (clone) {
            if (last_same != i - 1) monos[i - 1] = MonoClone(&monos[i - 1]);
            if (i == count - 1) monos[i] = MonoClone(&monos[i]);
        }
    }
    RemoveMonos(monos, count, remove, remove_size, clone);
    *new_size = count - remove_size;
}

/**
 * Sumuje listę jednomianów i tworzy z nich wielomian. Przejmuje na własność
 * pamięć wskazywaną przez @p monos. Jeśli @p clone @f$=@f$ <true>, nie
 * modyfikuje zawartości tablicy @p monos i jeśli jest to wymagane, wykonuje
 * pełne kopie jednomianów z tablicy @p monos. Jeśli @p clone @f$=@f$ <false>,
 * przejmuje na własność zawartość tablicy @p monos i może dowolnie modyfikować
 * zawartość tej pamięci.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @param[in] clone : określa czy zawartość tablicy @p monos jest przejmowana
 * na własność - jeśli <true> - nie jest przejmowana na własność
 * @return wielomian będący sumą jednomianów
 */
static Poly PolyFromMonos(size_t count, Mono *monos, bool clone) {
    size_t new_count;
    // Lista jednomianów jest sortowana malejąco względem wykładników.
    qsort(monos, count, sizeof(Mono), CompareMonos);

    JoinExponents(monos, count, &new_count, clone);
    assert(new_count != 0);
    count = new_count;

    RemoveZeros(monos, count, &new_count);

    Poly res = PolyFromArrSimplify(monos, new_count);
    return res;
}

/**
 * Sumuje listę jednomianów i tworzy z nich wielomian. Przejmuje na własność
 * pamięć wskazywaną przez @p monos i jej zawartość. Może dowolnie modyfikować
 * zawartość tej pamięci. Zakładamy, że pamięć wskazywana przez @p monos
 * została zaalokowana na stercie. Jeśli @p count lub @p monos jest równe zeru
 * (NULL), tworzy wielomian tożsamościowo równy zeru.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyOwnMonos(size_t count, Mono *monos) {
    if (count == 0 || !monos) {
        free(monos);
        return PolyZero();
    }
    else {
        return PolyFromMonos(count, monos, false);
    }
}

/**
 * Sumuje listę jednomianów i tworzy z nich wielomian.
 * Przejmuje na własność zawartość tablicy @p monos.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyAddMonos(size_t count, const Mono monos[]) {
    assert(count == 0 || monos != NULL);
    if (count == 0) return PolyZero();
    Mono *new_monos = malloc(count * sizeof(Mono));
    if (!new_monos) exit(1); // Błąd podczas alokacji pamięci.
    CopyMonos(new_monos, monos, count);

    return PolyOwnMonos(count, new_monos);
}

/**
 * Sumuje listę jednomianów i tworzy z nich wielomian. Nie modyfikuje zawartości
 * tablicy @p monos. Jeśli jest to wymagane, to wykonuje pełne kopie jednomianów
 * z tablicy @p monos. Jeśli @p count lub @p monos jest równe zeru (NULL),
 * tworzy wielomian tożsamościowo równy zeru.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyCloneMonos(size_t count, const Mono monos[]) {
    if (count == 0 || !monos) {
        return PolyZero();
    }
    Mono *new_monos = malloc(count * sizeof(Mono));
    if (!new_monos) exit(1); // Błąd podczas alokacji pamięci.
    CopyMonos(new_monos, monos, count);

    return PolyFromMonos(count, new_monos, true);
}

/**
 * Mnoży dwa wielomiany.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p * q@f$
 */
Poly PolyMul(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p->coeff * q->coeff);
    }
    else if (PolyIsCoeff(p)) {
        if (PolyIsZero(p)) return PolyZero();
        Poly p_extended = ExtendPoly(p);
        Poly res = PolyMul(&p_extended, q);
        PolyDestroy(&p_extended);
        return res;
    }
    else if (PolyIsCoeff(q)) {
        return PolyMul(q, p);
    }
    else {
        Mono monos[p->size * q->size];
        // Mnożymy każdy jednomian z [p->arr] z każdym jednomianem z [q->arr]
        // i dodajemy do siebie wyniki mnożeń.
        for (size_t i = 0; i < p->size; i++) {
            for (size_t j = 0; j < q->size; j++) {
                Poly curr_p = p->arr[i].p, curr_q = q->arr[j].p;
                Poly poly_mul = PolyMul(&curr_p, &curr_q);
                poly_exp_t exp_add = p->arr[i].exp + q->arr[j].exp;
                if (PolyIsZero(&poly_mul)) {
                    monos[i * q->size + j] = MonoFromPoly(&poly_mul, 0);
                }
                else {
                    monos[i * q->size + j] = MonoFromPoly(&poly_mul, exp_add);
                }
            }
        }
        return PolyAddMonos(p->size * q->size, monos);
    }
}

/**
 * Zwraca przeciwny wielomian.
 * @param[in] p : wielomian @f$p@f$
 * @return @f$-p@f$
 */
Poly PolyNeg(const Poly *p) {
    assert(p != NULL);
    Poly neg = PolyFromCoeff(-1);
    return PolyMul(p, &neg);
}

/**
 * Odejmuje wielomian od wielomianu.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p - q@f$
 */
Poly PolySub(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    Poly q_neg = PolyNeg(q);
    Poly res = PolyAdd(p, &q_neg);
    PolyDestroy(&q_neg);
    return res;
}

/**
 * Zwraca stopień wielomianu ze względu na zadaną zmienną (-1 dla wielomianu
 * tożsamościowo równego zeru). Zmienne indeksowane są od @p depth.
 * @param[in] p : wielomian
 * @param[in] var_idx : indeks zmiennej
 * @return stopień wielomianu @p p z względu na zmienną o indeksie @p var_idx
 */
static poly_exp_t PolyDegByHelper(const Poly *p, size_t var_idx, size_t depth) {
    if (PolyIsCoeff(p)) {
        if (PolyIsZero(p)) return -1;
        else return 0;
    }
    else if (depth == var_idx) {
        // Tablica [p->arr] przechowuje jednomian o największym wykładniku na
        // pozycji 0, ponieważ jest posortowana malejąco względem wykładników.
        return p->arr[0].exp;
    }
    else {
        // Stopień wielomianu ze względu na daną zmienną jest równy maksimum ze
        // stopni tworzących go jednomianów (ze względu na tę zmienną).
        poly_exp_t max_exp = -1;
        for (size_t i = 0; i < p->size; i++) {
            Poly curr_poly = p->arr[i].p;
            poly_exp_t curr_poly_deg = PolyDegByHelper(&curr_poly, var_idx, depth + 1);
            if (curr_poly_deg > max_exp) {
                max_exp = curr_poly_deg;
            }
        }
        return max_exp;
    }
}

/**
 * Zwraca stopień wielomianu ze względu na zadaną zmienną (-1 dla wielomianu
 * tożsamościowo równego zeru). Zmienne indeksowane są od 0.
 * Zmienna o indeksie 0 oznacza zmienną główną tego wielomianu.
 * Większe indeksy oznaczają zmienne wielomianów znajdujących się
 * we współczynnikach.
 * @param[in] p : wielomian
 * @param[in] var_idx : indeks zmiennej
 * @return stopień wielomianu @p p z względu na zmienną o indeksie @p var_idx
 */
poly_exp_t PolyDegBy(const Poly *p, size_t var_idx) {
    assert(p != NULL);
    return PolyDegByHelper(p, var_idx, 0);
}

/**
 * Zwraca stopień wielomianu (-1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] p : wielomian
 * @return stopień wielomianu @p p
 */
poly_exp_t PolyDeg(const Poly *p) {
    assert(p != NULL);
    if (PolyIsCoeff(p)) {
        if (PolyIsZero(p)) return -1;
        else return 0;
    }
    else {
        // Stopień wielomianu jest równy maksimum ze stopni tworzących go
        // jednomianów. Stopień jednomianu jest równy sumie jego wykładnika
        // i stopnia jego wielomianu.
        poly_exp_t max_exp = -1;
        for (size_t i = 0; i < p->size; i++) {
            Mono curr_mono = p->arr[i];
            poly_exp_t curr_poly_deg = PolyDeg(&curr_mono.p) + curr_mono.exp;
            if (curr_poly_deg > max_exp) {
                max_exp = curr_poly_deg;
            }
        }
        return max_exp;
    }
}

static bool MonoIsEq(const Mono *m, const Mono *n);

/**
 * Sprawdza równość dwóch wielomianów.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p = q@f$
 */
bool PolyIsEq(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return p->coeff == q->coeff;
    }
    else if (!PolyIsCoeff(p) && !PolyIsCoeff(q)) {
        if (p->size != q->size) return false;
        for (size_t i = 0; i < p->size; i++) {
            if (!MonoIsEq(&p->arr[i], &q->arr[i])) return false;
        }
        return true;
    }
    else {
        return false;
    }
}

/**
 * Sprawdza równość dwóch jednomianów.
 * @param[in] m : jednomian @f$m@f$
 * @param[in] n : jednomian @f$n@f$
 * @return @f$m = n@f$
 */
static bool MonoIsEq(const Mono *m, const Mono *n) {
    assert(m != NULL && n != NULL);
    if (m->exp != n->exp) return false;
    return PolyIsEq(&m->p, &n->p);
}

/**
 * Wylicza wartość wielomianu w punkcie @p x.
 * Wstawia pod pierwszą zmienną wielomianu wartość @p x.
 * W wyniku może powstać wielomian, jeśli współczynniki są wielomianami.
 * Wtedy zmniejszane są o jeden indeksy zmiennych w takim wielomianie.
 * Formalnie dla wielomianu @f$p(x_0, x_1, x_2, \ldots)@f$ wynikiem jest
 * wielomian @f$p(x, x_0, x_1, \ldots)@f$.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] x : wartość argumentu @f$x@f$
 * @return @f$p(x, x_0, x_1, \ldots)@f$
 */
Poly PolyAt(const Poly *p, poly_coeff_t x) {
    assert(p != NULL);
    if (x == 0) return PolyZero();
    if (PolyIsCoeff(p)) {
        return *p;
    }
    else {
        size_t monos_size = 0; // To jest zmienna przechowująca liczbę jednomianów
        // (niezredukowanych) po zastąpieniu zmiennych o indeksie 0 przez liczby.
        // Obliczanie [monos_size].
        for (size_t i = 0; i < p->size; i++) {
            Poly curr_poly = p->arr[i].p;
            if (PolyIsCoeff(&curr_poly)) {
                monos_size += 1;
            }
            else {
                monos_size += curr_poly.size;
            }
        }
        Mono monos[monos_size]; // To jest lista jednomianów, z których zostanie
        // stworzony wynikowy wielomian.
        size_t monos_idx = 0;
        for (size_t i = 0; i < p->size; i++) {
            Poly curr_poly = p->arr[i].p;
            if (PolyIsCoeff(&curr_poly)) {
                Poly p_mul = PolyFromCoeff(curr_poly.coeff * power(x, p->arr[i].exp));
                monos[monos_idx] = MonoFromPoly(&p_mul, 0);
                monos_idx++;
            }
            else {
                for (size_t j = 0; j < curr_poly.size; j++) {
                    Poly curr_p = curr_poly.arr[j].p;
                    Poly x_to_power = PolyFromCoeff(power(x, p->arr[i].exp));
                    Poly p_mul = PolyMul(&curr_p, &x_to_power);
                    if (PolyIsZero(&p_mul)) {
                        monos[monos_idx] = MonoFromPoly(&p_mul, 0);
                    }
                    else {
                        monos[monos_idx] = MonoFromPoly(&p_mul,
                                                        curr_poly.arr[j].exp);
                    }
                    monos_idx++;
                    PolyDestroy(&x_to_power);
                }
            }
        }
        return PolyAddMonos(monos_size, monos);
    }
}

/**
 * Podnosi wielomian do potęgi naturalnej.
 * @param p : wielomian @f$p@f$
 * @param n : potęga @f$n@f$
 * @return @f$p^n@f$
 */
Poly PolyPower(const Poly *p, poly_exp_t n) {
    assert(n >= 0);
    if (n == 0) return PolyFromCoeff(1);
    if (n == 1) return PolyClone(p);

    Poly q = PolyPower(p, n / 2);
    Poly q_squared = PolyMul(&q, &q);
    PolyDestroy(&q);

    if (n % 2 == 0) {
        return q_squared;
    }
    else {
        Poly res = PolyMul(p, &q_squared);
        PolyDestroy(&q_squared);
        return res;
    }
}

Poly MonoComposeHelper(const Mono *m, size_t k, const Poly q[], size_t depth, poly_exp_t *last_pow, Poly *last_pow_p);

/**
 * Zwraca złożenie wielomianu @p p z wielomianami @f$q_{depth}, q_{depth+1},
 * \ldots@f$. Zachowuje się tak jak PolyCompose() poza tym, że zmienne wielomianu
 * @p p są indeksowane od @p depth. Po dokładniejsze wytłumaczenie działania funkcji
 * zajrzyj do dokumentacji funkcji PolyCompose().
 * @param[in] p : wielomian @f$p@f$
 * @param[in] k : liczba wielomianów @f$q_i@f$
 * @param[in] q : lista wielomianów: @f$q_0, q_1, \ldots, q_{k-1}@f$
 * @param[in] depth : liczba, od której indeksowane są zmienne w @p p
 * @return @f$p(q_{depth}, q_{depth+1}, …)@f$
 */
Poly PolyComposeHelper(const Poly *p, size_t k, const Poly q[], size_t depth) {
    if (PolyIsCoeff(p)) return *p;
    else {
        Poly res = PolyZero();
        poly_exp_t last_pow = 0; // To jest ostatnia potęga, do której podnoszone
        // było [q[depth]]
        Poly last_pow_p = PolyFromCoeff(1); // To jest [q[depth]] podniesione
        // do potęgi [last_pow]
        for (size_t i = p->size; i-- > 0;) { // [i] maleje, aby [q[depth]]
            // podnoszone było do coraz wyższych potęg.
            Poly temp = MonoComposeHelper(&p->arr[i], k, q, depth, &last_pow, &last_pow_p);
            Poly new_res = PolyAdd(&res, &temp);
            PolyDestroy(&temp);
            PolyDestroy(&res);
            res = new_res;
        }
        PolyDestroy(&last_pow_p);
        return res;
    }
}

/**
 * Podstawia @f$q_{depth}@f$ pod zmienną @f$x_{depth}@f$ jednomianu @p m oraz
 * podstawia wynik funkcji PolyComposeHelper() działającą na wielomian będący
 * współczynnikiem jednomianu @p m pod ten wielomian, przy czym zmienne tego
 * wielomianu indeksowane są od @p depth @f$+ 1@f$.
 * @param[in] m : jednomian @f$p*x_{depth}^{exp}@f$
 * @param[in] k : @param[in] k : liczba wielomianów @f$q_i@f$
 * @param[in] q : lista wielomianów: @f$q_0, q_1, \ldots, q_{k-1}@f$
 * @param[in] depth : indeks zmiennej jednomianu
 * @param[in,out] last_pow : ostatnia potęga, do której podnoszone było
 * @p q[depth]
 * @param[in,out] last_pow_p : @p q[depth] podniesione do potęgi @p last_pow
 * @return @f$p(q_{depth+1}, q_{depth+2}, \ldots)*q_{depth}^{exp}@f$
 */
Poly MonoComposeHelper(const Mono *m, size_t k, const Poly q[], size_t depth, poly_exp_t *last_pow, Poly *last_pow_p) {
    if (depth >= k) {
        if (m->exp != 0) return PolyZero();
        else return PolyComposeHelper(&m->p, k, q, depth + 1);
    }
    else {
        Poly pow_1 = PolyPower(&q[depth],(m->exp - *last_pow));
        Poly pow_2 = PolyMul(last_pow_p, &pow_1);
        PolyDestroy(&pow_1);
        PolyDestroy(last_pow_p);
        *last_pow_p = pow_2;
        *last_pow = m->exp;
        Poly p = PolyComposeHelper(&m->p, k, q, depth + 1);

        Poly res = PolyMul(&pow_2, &p);
        PolyDestroy(&p);
        return res;
    }
}

/**
 * Zwraca złożenie wielomianu @p p z wielomianami @f$q_0, q_1, \ldots@f$ .
 * Niech @f$l@f$ oznacza liczbę zmiennych wielomianu @p p i niech te zmienne
 * są oznaczone odpowiednio @f$x_0, x_1, …, x_{l−1}@f$. Wynikiem złożenia jest
 * wielomian @f$p(q_0,q_1,q_2,…)@f$, czyli wielomian powstający przez podstawienie
 * w wielomianie @p p pod zmienną @f$x_i@f$ wielomianu @f$q_i@f$
 * dla @f$i=0,1,2,…,min(k,l)−1@f$. Jeśli @f$k<l@f$, to pod zmienne @f$x_k, …,
 * x_{l−1}@f$ podstawiane są zera.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] k : liczba wielomianów @f$q_i@f$
 * @param[in] q : lista wielomianów: @f$q_0, q_1, \ldots, q_{k-1}@f$
 * @return @f$p(q_0, q_1, q_2, …)@f$
 */
Poly PolyCompose(const Poly *p, size_t k, const Poly q[]) {
    return PolyComposeHelper(p, k, q, 0);
}