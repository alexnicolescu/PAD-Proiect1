# PAD-Proiect1
Clientul se poate conecta la server prin crearea unui cont sau pe baza unui cont existent. 
Dacă clientul alege crearea unui nou cont, acesta trebuie sa introducă un nume si o parola, daca numele exista deja, clientul trebuie să 
introducă alte date. Dacă clientul se conectează pe baza unui cont existent, trebuie de asemenea sa introducă numele si parola, daca 
acestea sunt greșite, se cere reintroducerea acestora.
După conectarea la server, fiecare client poate transmite un mesaj, acesta fiind primit de toti clientii conectați in acel moment la server,
inclusiv el. 
In client vor exista 2 procese, procesul fiu si procesul părinte. Procesul fiu este responsabil de citirea mesajelor de la ceilalti clienti,
iar procesul parinte de scrierea mesajelor.
Toti descriptorii clienților conectați la server se retin într-un fisier, iar pentru fiecare client se crează un thread in server ce se 
ocupa cu citirea mesajelor de la client si parcurgerea fișierului cu descriptori pentru transmiterea mesajelor.
Dacă clientul dorește deconectarea de la server, acesta trebuie sa transmită mesajul "exit", astfel se va trimite semnalul SIGUSR1 către
procesul fiu pentru a își închide descriptorul si se va termina urmând ca în părinte sa se execute functia pentru tratarea semnalului 
SIGCHLD. 
In server, daca mesajul clientului este exit, se apelează functia pentru stergerea descriptorul corespunzător clientului.
Astfel clientul se va deconecta de la server, urmând ca restul clienților sa isi continue conversatia.
