import socketio
import socket
import sys



terminate_thread = False

# Créer un objet SocketIO
sio = socketio.Client()

# Définir les détails de la connexion au serveur SocketIO
server_url = 'http://localhost:3001'

# Créer une connexion socket pour la communication avec le programme C
c_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
c_socket.connect(('127.0.0.1', 12345))  # Remplacez par l'adresse et le port du programme C

# Fonction appelée lorsqu'une connexion est établie avec le serveur SocketIO
@sio.event
def connect():
    print('Connexion établie avec le serveur')

# Fonction appelée lorsqu'un message est reçu
@sio.event
def message(data):
    print(f'Message reçu du serveur Node : {data}')
    
    # Envoyer le message au programme C
    c_socket.send(data['message'].encode('utf-8'))


# Fonction appelée lorsque le code rest reçu
@sio.event
def game_code(data):
    print(f'Code reçu : {data}')
    # Envoyer le code au programme C
    code_message = f"code={data['gameCode']}"
    c_socket.send(code_message.encode('utf-8'))



# Fonction appelée lorsqu'une déconnexion se produit
@sio.event
def disconnect():
    global terminate_thread  # Utilisez la variable globale

    print('Déconnexion du serveur')
    if not terminate_thread:
        c_socket.send('Deconnexion'.encode('utf-8'))

# Fonction pour écouter en permanence les messages du programme C
def listen_to_c():
    global terminate_thread  # Utilisez la variable globale

    while not terminate_thread:
        data = c_socket.recv(1024)
        if not data:
            break
        received_message = data.decode("utf-8")
        print(f'Message reçu du programme C : {received_message}')
        # Analyser le message et prendre des mesures appropriées
        if "create-game" in received_message:
            parts = received_message.split('|')
            if len(parts) >= 2:
                player_name = parts[1]
                print(f'Le joueur {player_name} souhaite créer une partie')
                # Émettre l'événement 'create-game' après la connexion
                sio.emit('create-game', player_name)
        elif "close" in received_message:
            #Fermer le programme python
            print("Le jeu s'est fermé.")
            terminate_thread = True  # Fermer le Thread

        

# Fonction principale pour la gestion de la connexion
def main():
    global terminate_thread  # Utilisez la variable globale


    # Connecter le client au serveur SocketIO
    sio.connect(server_url)
    
   
    
    try:
        # Démarrer un thread ou un processus pour écouter les messages du programme C
        # Cela permet de gérer simultanément les communications SocketIO et du programme C
        import threading
        c_listener_thread = threading.Thread(target=listen_to_c)
        c_listener_thread.start()

        while not terminate_thread:
            # Enregistrer le gestionnaire d'événements pour l'événement 'test-message'
            sio.on('test-message', message)
            sio.on('game-code', game_code)

            pass  # Boucle infinie pour maintenir le programme Python en cours d'exécution
    except KeyboardInterrupt:
        print("Arrêt du programme Python")

    # Déconnecter le client
    sio.disconnect()
    c_socket.close()
    
    # Attendre que le thread de l'écoute se termine
    c_listener_thread.join()
    sys.exit(0)
    

# Exécuter la fonction principale
if __name__ == '__main__':
    main()
