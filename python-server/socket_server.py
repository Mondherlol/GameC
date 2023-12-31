import socketio
import socket
import sys
import os


VERSION=1.1

terminate_thread = False

# Créer un objet SocketIO
sio = socketio.Client()

# Définir les détails de la connexion au serveur SocketIO
server_url = 'http://localhost:3001'

nom_fichier = 'SERVER_URL.txt'


try:
    # Ouvrir le fichier en mode lecture
    with open(nom_fichier, 'r') as fichier:
        # Lire la première ligne et la stocker dans une variable
        server_url = fichier.readline()
        print(f"La server_url lue depuis le fichier est : {server_url}")

except FileNotFoundError:
    print(f"Le fichier '{nom_fichier}' n'a pas été trouvé.")

except IOError as e:
    print(f"Une erreur s'est produite lors de la lecture du fichier : {e}")
    

# Créer une connexion socket pour la communication avec le programme C
c_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def connect_to_game():
    global terminate_thread  # Utilisez la variable globale
    try:
        c_socket.connect(('127.0.0.1', 12345))  # Adresse et port du serveur local C
        print('✅ Connexion établie avec le jeu ')
    except ConnectionRefusedError:
        print("❌ La connexion au jeu a échoué. Veuillez vous assurer que le jeu est bien lancé.")
        terminate_thread = True
  
def connect_to_server():
    global terminate_thread  # Utilisez la variable globale
    try:
        sio.connect(server_url)          
    except :
        print("❌ La connexion au serveur a échoué. Réessayez plus tard... 😥")
        terminate_thread = True
        
# Fonction appelée lorsqu'une connexion est établie avec le serveur SocketIO
@sio.event
def connect():
    print('✅ Connexion établie avec le serveur')

# Fonction appelée lorsqu'un message est reçu
@sio.event
def test_message(data):
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

@sio.event
def visitor_joined(data):
    print(f"Le visiteur {data['visitorName']} avec l'ID {data['socketId']} a rejoint la partie")
    new_visitor = f"new_visitor={data['visitorName']}&socketId={data['socketId']}"
    c_socket.send(new_visitor.encode('utf-8'))

@sio.event
def new_enemy(data):
    print(f"Le visiteur {data['visitorName']} avec l'ID {data['socketId']} a envoyer l'ennemi  {data['enemyType']} ")
    new_enemy = f"new_enemy={data['enemyType']}&socketId={data['socketId']}"
    c_socket.send(new_enemy.encode('utf-8'))
    
@sio.event
def visitor_leaved(data):
    print(f"Le visiteur {data['visitorName']} avec l'ID {data['socketId']} a quitté la partie")
    leaving_visitor = f"leaving_visitor={data['socketId']}"
    c_socket.send(leaving_visitor.encode('utf-8'))


# Fonction appelée lorsqu'une déconnexion se produit
@sio.event
def disconnect():
    global terminate_thread  # Utilisez la variable globale

    print('👋 Déconnexion du serveur')
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
        elif "game-statut" in received_message:
            parts = received_message.split('|')
            if len(parts) >= 2:
                game_statut_str = parts[1].strip().split()[0].lower()  # Obtenez le premier mot après le pipe
                
                game_statut = True if game_statut_str == "true" else False
                socket_id_killer = parts[2].strip() if len(parts) >= 3 else ""  
                
                print(f'Une partie vient de {"commencer" if game_statut else "se terminer"}')
                # Créer un dictionnaire pour regrouper les paramètres
                data_to_emit = {'game_statut': game_statut, 'socket_id_killer': socket_id_killer}

                # Prévenir le serveur 
                sio.emit('game-statut-changed', data_to_emit)
                
        elif "close" in received_message:
            #Fermer le programme python
            print("Le jeu s'est fermé.")
            terminate_thread = True  # Fermer le Thread

        

# Fonction principale pour la gestion de la connexion
def main():
    global terminate_thread  # Utilisez la variable globale
    print(f"===SERVEUR PYTHON version {VERSION}====\n")

    # Connecter le client au serveur SocketIO
    connect_to_server()
    
    # Connecter le client au serveur C local (Jeu)
    connect_to_game()
    
   
    
    try:
        # Démarrer un thread ou un processus pour écouter les messages du programme C
        # Cela permet de gérer simultanément les communications SocketIO et du programme C
        import threading
        c_listener_thread = threading.Thread(target=listen_to_c)
        c_listener_thread.start()

        while not terminate_thread:
            # Enregistrer le gestionnaire d'événements pour l'événement 'test-message'
            sio.on('test-message', test_message)
            sio.on('game-code', game_code)
            sio.on('visitor-joined', visitor_joined)
            sio.on('visitor-leaved', visitor_leaved)
            sio.on('new-enemy', new_enemy)

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
