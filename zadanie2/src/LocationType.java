/**
 * MoĹźliwe typy lokacji znajdujÄcych siÄ w labiryncie.
 */
public enum LocationType {
    /**
     * Ĺciana. Odebranie tego typu lokacji oznacza niestety poraĹźkÄ. Program
     * Ĺşle przetwarzaĹ otrzymywane wyniki i trafiĹ do wnÄtrza Ĺciany.
     */
    WALL,
    /**
     * WyjĹcie z labiryntu. Sukces!
     */
    EXIT,
    /**
     * PrzejĹcie, generalnie pusta przestrzeĹ pozwalajÄca na przemieszczanie
     * siÄ w labiryncie
     */
    PASSAGE;
}