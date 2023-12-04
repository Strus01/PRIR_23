<h4>Idea zadania</h4>

<p>
Budowa wielu histogramów jednocześnie.
</p>

<h4>Na czym polega problem?</h4>

<p>Tworzymy system umożliwiający generowanie wielu histogramów w tym samym czasie przez wielu
równoczesnych użytkowników.</p>

<p>System ma być aplikacją rozproszoną, gdzie serwer ma odpowiadać za budowę histogramów a
użytkownicy (klienci) za dostarczanie danych.</p>

<h4>Komunikacja</h4>

<ul>
<li>Klient tworzy histogram. Uzyskuje unikalny identyfikator histogramu.
</li><li>Klient przekazuje dane dla histogramu. Każdorazowa przekazywany jest identyfikator histogramu, aby
powiązać daną z histogramem.
</li><li>Klient odbiera histogram o wskazanym numerze identyfikacyjnym.
</li></ul>

<p>Powyższe może być wykonywane przez <b>wielu klientów jednocześnie</b>.</p>

<h4>Założenia</h4>

<ul>
<li>Przed pierwszym użyciem serwis zostanie zarejestrowany (bind)
</li><li>Przed przekazaniem pierwszej danej utworzony zostanie histogram
</li><li>Przekazywane przez klienta dane będą pasować do liczby kubełków danego histogramu (dozwolone od 0 do bins-1)
</li><li>Metoda getHistogram nie zostanie wykonana przed utworzeniem histogramu
</li><li>Generalnie, klient będzie posługiwał się wyłącznie poprawnymi numerami histogramów.
</li></ul>

<h4>Dostarczanie rozwiązania</h4>

<p>Proszę o dostarczenie kodu <b>źródłowego</b> klasy <code class="expectedclass">RMIHistogram</code>.
W klasie można umieścić własne metody i pola. Klasa ma implementować interfejsy <code>RemoteHistogram</code>. Sama część usługowa RMI to pierwszy z interfejsów, drugi odpowiada wyłącznie
za możliwość przekazania do Państawa kodu zlecenia rejestracji w rmiregistry.
i <code>Binder</code></p>

<p>Uwaga: proszę nie tworzyć samodzielnie rmiregistry w swoim programie!</p>

