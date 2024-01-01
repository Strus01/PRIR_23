/**
 * Kierunki ruchu
 */
public enum Direction {
    /**
     * Na północ
     */
    NORTH {
        @Override
        public Location step(Location currentLocation) {
            return new Location(currentLocation.col(), currentLocation.row() + 1);
        }
    },
    /**
     * Na południe
     */
    SOUTH {
        @Override
        public Location step(Location currentLocation) {
            return new Location(currentLocation.col(), currentLocation.row() - 1);
        }
    },
    /**
     * Na wschód
     */
    EAST {
        @Override
        public Location step(Location currentLocation) {
            return new Location(currentLocation.col() + 1, currentLocation.row());
        }

    },
    /**
     * Na zachód
     */
    WEST {
        @Override
        public Location step(Location currentLocation) {
            return new Location(currentLocation.col() - 1, currentLocation.row());
        }
    };

    /**
     * Zwraca połoźenie po wykonaniu ruchu w danym kierunku
     *
     * @param currentLocation aktualne połoźenie
     * @return następne połoźenie
     */
    abstract public Location step(Location currentLocation);
}