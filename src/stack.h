/** @file
  Interfejs stosu przechowującego wielomiany

  @authors Izabela Ożdżeńska <io417924@students.mimuw.edu.pl>
  @date 2021
*/

#ifndef GAMMA_STACK_H
#define GAMMA_STACK_H

#include "poly.h"

/**
 * To jest struktura przechowująca element stosu wielomianów.
 */
typedef struct StackNode {
    Poly p;                 ///< wielomian
    struct StackNode *next; ///< kolejny element stosu
} StackNode;

/**
 * To jest typ przechowujący stos wielomianów.
 */
typedef StackNode* Stack;

/**
 * Zwraca pusty stos.
 * @return pusty stos
 */
static inline Stack create(void) {
    return NULL;
}

/**
 * Sprawdza czy stos ma co najmniej @p n elementów.
 * @param[in] top : stos (in. wierzchni element stosu)
 * @param[in] n : liczba elementów
 * @return @f$0@f$, jeśli stos ma mniej niż @p n elementów; @f$1@f$
 * w przeciwnym przypadku
 */
bool hasnElements(const StackNode *top, size_t n);

/**
 * Zwraca @p n -ty element stosu. Elementy indeksowane są od @f$0@f$.
 * W przypadku gdy na stosie jest mniej niż @p n elementów, program kończy
 * działanie.
 * @param[in] top : stos (in. wierzchni element stosu)
 * @param[in] n : indeks elementu
 * @return @p n -ty element stosu
 */
Poly nthElement(const StackNode *top, size_t n);

/**
 * Dodaje wielomian na wierzch stosu.
 * @param[in,out] top : stos (in. wierzchni element stosu)
 * @param[in] p : wielomian
 */
void push(StackNode **top, Poly p);

/**
 * Zwraca wierzchni element stosu. Usuwa element ze stosu. W przypadku gdy
 * stos jest pusty, program kończy działanie.
 * @param[in,out] top : stos (in. wierzchi element stosu)
 * @return stos bez wierzchniego elementu
 */
Poly pop(StackNode **top);

#endif //GAMMA_STACK_H
