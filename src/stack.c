/** @file
  Implementacja stosu przechowującego wielomiany

  @authors Izabela Ożdżeńska <io417924@students.mimuw.edu.pl>
  @date 2021
*/

#include <malloc.h>
#include "poly.h"
#include "stack.h"

/**
 * Tworzy nowy element stosu. Ustawia jako parametr zadany wielomian.
 * @param[in] p : wielomian
 * @return element stosu z zadanym wielomianem
 */
static StackNode* newNode(Poly p) {
    StackNode *stackNode = malloc(sizeof(StackNode));
    stackNode->p = p;
    stackNode->next = NULL;
    return stackNode;
}

/**
 * Sprawdza czy stos ma co najmniej @p n elementów.
 * @param[in] top : stos (in. wierzchni element stosu)
 * @param[in] n : liczba elementów
 * @return @f$0@f$, jeśli stos ma mniej niż @p n elementów; @f$1@f$
 * w przeciwnym przypadku
 */
bool hasnElements(const StackNode *top, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (!top) return false;
        top = top->next;
    }
    return true;
}

/**
 * Zwraca @p n -ty element stosu. Elementy indeksowane są od @f$0@f$.
 * W przypadku gdy na stosie jest mniej niż @p n elementów, program kończy
 * działanie.
 * @param[in] top : stos (in. wierzchni element stosu)
 * @param[in] n : indeks elementu
 * @return @p n -ty element stosu
 */
Poly nthElement(const StackNode *top, size_t n) {
    assert(hasnElements(top, n + 1));
    for (size_t i = 0; i < n; i++) {
        top = top->next;
    }
    return top->p;
}

/**
 * Dodaje wielomian na wierzch stosu.
 * @param[in,out] top : stos (in. wierzchni element stosu)
 * @param[in] p : wielomian
 */
void push(StackNode **top, Poly p) {
    StackNode *stackNode = newNode(p);
    stackNode->next = *top;
    *top = stackNode;
}

/**
 * Zwraca wierzchni element stosu. Usuwa element ze stosu. W przypadku gdy
 * stos jest pusty, program kończy działanie.
 * @param[in,out] top : stos (in. wierzchi element stosu)
 * @return stos bez wierzchniego elementu
 */
Poly pop(StackNode **top) {
    assert(hasnElements(*top, 1));
    StackNode *temp = *top;
    *top = (*top)->next;
    Poly popped = temp->p;
    free(temp);

    return popped;
}

