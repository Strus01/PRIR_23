/**
 * Interfejs obiektu oczekujÄcego na wynik eksploracji lokacji labiryntu.
 */
public interface ResultListener {
    /**
     * Metoda wykonywana po zakoĹczeniu ekspoloracji lokacji.
     *
     * @param result rezultat ekspoloracji lokacji
     */
    public void result(Result result);
}