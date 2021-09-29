/** @file
  Interfejs kalkulatora wykonującego działania na wielomianach

  @authors Izabela Ożdżeńska <io417924@students.mimuw.edu.pl>
  @date 2021
*/

#ifndef GAMMA_CALC_PARSE_H
#define GAMMA_CALC_PARSE_H

#include "poly.h"
#include "stack.h"

/**
 * To jest typ reprezentujący opcję (rodzaj) polecenia podawanego w kalkulatorze.
 * Wykonanie polecenia z daną opcją skutkuje wykonaniem pewnych akcji na stosie
 * wielomianów.
 */
typedef enum Option{
    ZERO,       ///< wstawia na wierzchołek stosu wielomian tożsamościowo równy
                ///< zeru
    IS_COEFF,   ///< sprawdza, czy wielomian na wierzchołku stosu jest
                ///< współczynnikiem – wypisuje na standardowe wyjście 0 lub 1
    IS_ZERO,    ///< sprawdza, czy wielomian na wierzchołku stosu jest
                ///< tożsamościowo równy zeru – wypisuje na standardowe wyjście
                ///< 0 lub 1
    CLONE,      ///< wstawia na stos kopię wielomianu z wierzchołka
    ADD,        ///< dodaje dwa wielomiany z wierzchu stosu, usuwa je i wstawia
                ///< na wierzchołek stosu ich sumę
    MUL,        ///< mnoży dwa wielomiany z wierzchu stosu, usuwa je i wstawia
                ///< na wierzchołek stosu ich iloczyn
    NEG,        ///< neguje wielomian na wierzchołku stosu
    SUB,        ///< odejmuje od wielomianu z wierzchołka wielomian pod
                ///< wierzchołkiem, usuwa je i wstawia na wierzchołek stosu
                ///< różnicę
    IS_EQ,      ///< sprawdza, czy dwa wielomiany na wierzchu stosu są równe –
                ///< wypisuje na standardowe wyjście 0 lub 1
    DEG,        ///< wypisuje na standardowe wyjście stopień wielomianu
                ///< (−1 dla wielomianu tożsamościowo równego zeru)
    DEG_BY,     ///< wypisuje na standardowe wyjście stopień wielomianu ze
                ///< względu na zmienną o numerze podanym jako argument
                ///< (−1 dla wielomianu tożsamościowo równego zeru)
    AT,         ///< wylicza wartość wielomianu w punkcie podanym jako argument,
                ///< usuwa wielomian z wierzchołka i wstawia na stos wynik
                ///< operacji
    PRINT,      ///< wypisuje na standardowe wyjście wielomian z wierzchołka
                ///< stosu
    POP,        ///< usuwa wielomian z wierzchołka stosu
    COMPOSE,    ///< wykonuje złożenie wielomianu z wierzchu stosu z @f$k@f$
                ///< kolejnymi wielomianami z wierzchu stosu, usuwa te @f$k+1@f$
                ///< wielomianów ze stosu i wstawia wynik złożenia na
                ///< wierzchołek stosu (więcej o operacji złożenia wielomianów
                ///< przeczytasz w dokumentacji funkcji PolyCompose() z pliku
                ///< poly.h)
    add_poly,   ///< dodaje wielomian podany jako argument w odpowiednim
                ///< formacie (patrz: correctPoly()) na wierzchołek stosu
    error       ///< nie wykonuje żadnych akcji
} Option;

/**
 * To jest struktura reprezentująca polecenie. Polecenie składa się z opcji
 * polecenia i, opcjonalnie, z argumentu. Polecenia z opcją <AT>, <DEG_BY>,
 * <COMPOSE> oraz <add_poly> są poleceniami z argumentem. Pozostałe polecenia
 * są bezargumentowe.
 */
typedef struct Command {
    Option opt; ///< opcja polecenia
    /**
     * To jest unia przechowująca argument polecenia.
     */
    union {
        poly_coeff_t at_arg;        ///< argument polecenia z opcją <AT>
        unsigned long deg_arg;      ///< argument polecenia z opcją <DEG_BY>
        unsigned long compose_arg;  ///< argument polecenia z opcją <COMPOSE>
        Poly p;                     ///< argument polecenia z opcją <add_poly>
    };
} Command;

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
void GetInput(void);

#endif //GAMMA_CALC_PARSE_H