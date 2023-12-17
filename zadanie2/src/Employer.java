import java.util.List;

/**
 * Interfejs zleceniodawcy zadaĹ eksporacji labiryntu.
 */
public interface Employer {
    /**
     * Metoda ustawia dostÄp do obiektu umoĹźliwiajÄcego zlecanie eksploracji
     * wskazanych lokacji labiryntu.
     *
     * @param order narzÄdzie do eksploracji labiryntu
     */
    public void setOrderInterface(OrderInterface order);

    /**
     * Rozpoczyna siÄ poszukiwanie wyjĹcia. Do metody przekazywane jest poczÄtkowe
     * poĹoĹźenie w labiryncie, od ktĂłrego zaczyna siÄ poszukiwanie. Wraz z
     * poĹoĹźeniem przekazywana jest informacji o moĹźliwych kierunkach, w jakich
     * moĹźna siÄ przemieĹciÄ. Metoda zostanie wykonana w innym wÄtku niĹź ten, ktĂłry
     * wykonaĹ setOrderInterface.
     *
     * @param startLocation     pozycja startowa
     * @param allowedDirections dozwolone kierunki dalszej ekspoloracji labiryntu
     * @return poĹoĹźenie, w ktĂłrym znajduje siÄ wyjĹcie z labiryntu
     */
    public Location findExit(Location startLocation, List<Direction> allowedDirections);
}