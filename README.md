# Syspro2
Implementation of a dropbox-like app using named pipes


Όνομα: Διανέλλος Στέφανος
ΑΜ: 1115201500036
ΠΡΟΓΡΑΜΜΑΤΙΣΜΟΣ ΣΥΣΤΗΜΑΤΟΣ
ΕΡΓΑΣΙΑ 2
Σύντομη επεξήγηση:
Το πρόγραμμα μεταγλωτίζεται με την εντολή: make.
Το πρόγραμμα ξεκινάει ελέγχοντας τις παραμέτρους εισόδου. Στη συνέχεια καλείται
μία συνάρτηση(checkDir) η οποία ελέγχει το common dir για τους ήδη υπάρχοντες
clients( έλεγχος για .id αρχεία). Αν βρεθεί κάποιο id διαφορετικό από αυτό του
συγκεκριμένου client ξεκινάει να κάνει τις υπόλοιπες δουλειές. Στην περίπτωση που
είναι ο πρώτος client ή για κάθε client που έχει τελειώσει με τη μεταφορά αρχείων, η
αναμονή και η ειδοποίηση για την εμφάνιση επόμενων client γίνεται με τη χρήση της
inotify. Με το που βρεθεί καινούριος client τότε οι ήδη υπάρχοντες clients ,αφού
μπουν στην checkDir και inotify αντίστοιχα, φτιάχνουν τα αντίστοιχα pipes.
Δημιουργώ δύο διεργασίες μία για write και μία για read στο pipe.
Στη write διεργασία έχω δημιουργήσει μία συνάρτηση(traverse) η οποία καιε traverse
το input dir και για κάθε αρχείο που βρίσκει το γράφει στο pipe με τη μορφή που έχει
δοθεί στην εκφώνηση.Μόλις βρει και το τελευταίο αρχείο βγαίνει από τη συνάρτηση
και γράφει 00.
Στη διεργασία υπάρχει ένας ατέρμων βρόγχος. Η συνθήκη για να βγει από αυτόν τον
βρόγχο είναι να διαβαστεί το 00.
Στην parent διεργασία ανάλογα με το exit status τον παιδιών υλοποιεί τα ζητούμενα
της εκφώνησης.
