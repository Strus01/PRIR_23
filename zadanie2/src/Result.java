import java.util.List;

/**
 * Interfejs rezultatu eksploracji pomieszczenia.
 */
public interface Result {
    /**
     * Numer zlecenia, ktĂłrego rezultat dotyczy
     *
     * @return numer zlecenia
     */
    public int orderID();

    /**
     * Typ lokacji
     *
     * @return typ lokacji
     */
    public LocationType type();

    /**
     * MoĹźliwe kierunki ruchu z danej lokacji. Sensowna wartoĹÄ zwracana jest w
     * przypadku lokacji typu PASSAGE. W innych przypadkach zwracana jest pusta
     * lista. Wynik jest typu read-only.
     *
     * @return lista kierunkĂłw
     */
    public List<Direction> allowedDirections();
}