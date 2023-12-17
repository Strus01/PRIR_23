/**
 * Interfejs umoĹźliwiajÄcy zlecenie zbadania lokacji.
 */
public interface OrderInterface {

    /**
     * Metoda pozwala na ustawienie obiektu, do ktĂłrego przekazywane bÄdÄ wyniki
     * eksploracji pomieszczeĹ. Metoda musi zostaÄ jednokrotnie wykonana przed
     * pierwszym zleceniem.
     *
     * @param listener obiekt oczekujÄcy na wyniki eksploracji
     */
    public void setResultListener(ResultListener listener);

    /**
     * Zlecenie eksploracji wskazanej lokacji labiryntu. Metoda zapisuje zlecenie i
     * koĹczy pracÄ. Zlecenie wykonane zostanie w dowolnym terminie.
     *
     * @param location poĹoĹźenie w labiryncie do zbadania
     * @return unikalny identyfikator zlecenia.
     */
    public int order(Location location);
}