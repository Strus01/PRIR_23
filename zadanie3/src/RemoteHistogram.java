import java.rmi.Remote;
import java.rmi.RemoteException;

/**
 * Interfejs zdalnego serwisu tworzącego histogramy.
 */
public interface RemoteHistogram extends Remote {
    /**
     * Utworzenie histogramu o określonej liczbie kubełków.
     *
     * @param bins liczba kubełków
     * @return unikalny numer identyfikujący histogram
     * @throws RemoteException
     */
    public int createHistogram(int bins) throws RemoteException;

    /**
     * Dodanie danej do histogramu o wskazanym identyfikatorze.
     *
     * @param histogramID identyfikator histogramu, do którego należy dodać value
     * @param value       wartość do dodania do histogramu histogramID
     * @throws RemoteException
     */
    public void addToHistogram(int histogramID, int value) throws RemoteException;

    /**
     * Pobranie histogramu o wskazanym identyfikatorze.
     *
     * @param histogramID identyfikator histogramu
     * @return tablica o rozmiarze bins z histogramem
     * @throws RemoteException
     */
    public int[] getHistogram(int histogramID) throws RemoteException;

}