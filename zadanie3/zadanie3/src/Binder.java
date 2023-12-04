/**
 * Interfejs rejestracji usługi RMI.
 */
public interface Binder {
    /**
     * Rejestruje w rmiregistry pod podaną nazwą serwer usługi. Usługa rmiregistry
     * będzie uruchomiona pod adresem localhost:1099. <b>Metoda nie może
     * samodzielnie uruchamiać rmiregistry!</b>
     *
     * @param serviceName oczekiwana nazwa usługi w rmiregistry
     */
    public void bind(String serviceName);
}