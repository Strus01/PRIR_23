<h4>Idea zadania</h4>

<p>
Odszukać wyjście z nieznanego labiryntu. 
</p>

<h4>Na czym polega problem?</h4>

<p>Rozpoczynamy wędrówkę po nieznanym labiryncie. Labirynt posiada wiele dróg rozmieszczonych na jednym poziomie planszy do gry. 
Plansza podzielona jest na kwadratowe pola (lokacje) o tym samym rozmiarze.
Niektóre z dróg są ślepe. Są i takie, które wprowadzają pętle. 
</p>

<p>Dodatkowo, do samego labiryntu nie będzie bezpośredniego dostępu. Operacje badania labiryntu można 
będzie zlecić mojemu oprogramowaniu, będzie ono jednak działać wielowątkowo i asynchronicznie.</p>


<p>Lokacja umieszczona w lewym-dolnym rogu labiryntu ma pozycję (row=0, col=0).

</p><h4>Poszukiwanie wyjścia</h4>

<p>Poszukując wyjścia należy przestrzegać następujących reguł:</p>

<ol>
<li>Nie wolno wielokrotnie zlecić eksploracji tej samej pozycji labiryntu.
</li><li>Nie wolno zlecić badania pozycji, która nie sąsiaduje bezpośrednio z żadną z wcześniej zbadanych.
Chodzi o najbliższe sąsiedztwo w poziomie i pionie (każde położenie ma czterech najbliższych sąsiadów).
Można w ten sposób trafić do wnętrza ściany, co jest niedozwolone!
</li><li>Labirynt należy eksplorować w wielu miejscach jednocześnie. Np. jeśli wiemy, że 
do danej pozycji w labiryncie dotarliśmy idąc na północ i uzyskaliśmy informację, że
sąsiaduje ona z pomieszczeniami na południe (droga powrotna), wschód i zachód, to dla pomieszczeń na wschód i zachód 
należy od razu uruchomić dwa zlecenia eksploracji (oczywiście, o ile pomieszczenia te nie zostały wcześniej zbadane).
</li><li>Generalnie, zawsze należy zlecać eksplorację wszystkich pomieszczeń, które zgodnie z wcześniejszymi
punktami, eksplorować już można.
</li></ol>

<h4>Informacja zwrotna</h4>

<p>Zlecenia będą realizowane "w tle", ale czas realizacji jest ogólnie nieznany. Zlecenia późniejsze
mogą zakończyć się wcześniej. Zlecenia mogą zakończyć się nawet w tej samej chwili.
Każde zlecenie zostanie zrealizowane.
</p>

<p>Należy się spodziewać, że każde wywołanie metody <code>result</code>
z intefejsu <code>ResultListener</code> będzie wykonane innym wątkiem.</p>

<h4>Przegrana</h4>

<p>Powody porażki:</p>

<ul>
<li>Nie zostanie znalezione wyjście. Czas poszukiwań będzie ograniczony, jednak limit będzie na tyle 
duży, aby poprawnie działający program nie miał problemu ze znalezieniem rozwiązania.</li>
<li>Kontynuowanie poszukiwań po odkryciu wyjścia. To oczywiście z uwzględnieniem "czasu reakcji"
związanym z odebraniem i przetworzeniem informacji o odnalezieniu wyjścia.
</li><li>Nieprzestrzeganie reguł poszukiwania wyjścia, w szczególności praca sekwencyjna!
</li><li>Nieodbieranie przekazanych informacji
</li><li>Wskazanie błędnej lokacji w labiryncie i poza nim
</li><li>Trafienie na lokację typu "WALL". Coś takiego nie ma prawa się zdarzyć. Wynik eksploracji
zawiera informację o dozwolonych kierunkach ruchu - należy jej używać!
</li><li>Używanie CPU w czasie, gdy program nie ma nic do zrobienia, czyli, gdy oczekuje na 
rezultat zleceń. Nieużywane wątki muszą być usypiane!
</li></ul>

<h4>Obserwator</h4>

<p>Ponieważ nie wiadomo kiedy zlecenie zostanie zrealizowane przez mój kod, a metod 
pozwalających na realizację "odpytywania" brak, pozostaje użycie wzorca obserwator.
Obiekt odbierający zlecenia musi zostać przez Państwa dostarczony przed 
pierwszym zleceniem. Każde zlecenie będzie otrzymywać unikalny numer. 
Obserwator będzie otrzymywać informacje o wskazanej lokacji wraz z numerem zlecenia. 
Dzięki temu będzie możliwe powiązanie wyniku ze zleceniem.</p>

<h4>Komunikacja</h4>

<ul>
<li>Z Państwa kodem komunikuje się wyłącznie poprzez interfejs Employer.
</li><li>Przekazuję najpierw obiekt zgodny OrderInterface
</li><li>Następnie wykonuję w osobnym wątku metodę start(), do której trafia położenie początkowe i
kierunki w jakich można rozpocząć poszukiwanie wyjścia. Położenie początkowe na pewno jest typu 
PASSAGE.
</li><li>Państwa kod przekazuje do mojego za pomocą setResultListener obiekt zgodny z ResultListener.
</li><li>Teraz Państwa program może zlecać (metoda order) badanie kolejnych lokacji
</li><li>Mój program w dowolnym momencie przekaże wynik zlecenia
</li><li>Ostatnie kroki powtarzane są aż do odnalezienia wyjścia.
</li><li>Odnalezienie wyjścia powinno zakończyć prace metody start().
</li></ul>

<h4>Dostarczanie rozwiązania</h4>

<p>Proszę o dostarczenie kodu <b>źródłowego</b> klasy <code class="expectedclass">ParallelEmployer</code>.
W klasie można umieścić własne metody i pola. Klasa 
ma implementować interfejs <code>Employer</code>.
</p>

<p>Plik z rozwiązaniem może zawierać inne klasy, ale tylko 
klasa ParallelEmployer może być publiczna.</p>

<p>Kodu, który sam dostarczam nie wolno modyfikować. W trakcie testów używany będzie w 
takiej formie, w jakiej został udostępniony.</p>

<p>Kodu, który sam udostępniam proszę nie dołączać do rozwiązań. Będzie dostępny w trakcie testów.</p>

<p>Programy będą testowane za pomocą Java w wersji 17.</p>

<h4>Dodatek</h4>

<p>Dodałem rekord <code>Location</code>, który używany jest przez typ wyliczeniowy <code>Direction</code>.
Można zastosować we własnym rozwiązaniu. Można i nie stosować...</p>
