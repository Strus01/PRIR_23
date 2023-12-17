/**
 * Kierunki ruchu
 */
public enum Direction {
    /**
     * Na pĂłĹnoc
     */
    NORTH {
        @Override
        public Location step(Location currentLocation) {
            return new Location(currentLocation.col(), currentLocation.row() + 1);
        }
    },
    /**
     * Na poĹudnie
     */
    SOUTH {
        @Override
        public Location step(Location currentLocation) {
            return new Location(currentLocation.col(), currentLocation.row() - 1);
        }
    },
    /**
     * Na wschĂłd
     */
    EAST {
        @Override
        public Location step(Location currentLocation) {
            return new Location(currentLocation.col() + 1, currentLocation.row());
        }

    },
    /**
     * Na zachĂłd
     */
    WEST {
        @Override
        public Location step(Location currentLocation) {
            return new Location(currentLocation.col() - 1, currentLocation.row());
        }
    };

    /**
     * Zwraca poĹoĹźenie po wykonaniu ruchu w danym kierunku
     *
     * @param currentLocation aktualne poĹoĹźenie
     * @return nastÄpne poĹoĹźenie
     */
    abstract public Location step(Location currentLocation);
}