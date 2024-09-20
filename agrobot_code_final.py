import firebase_admin
from firebase_admin import credentials, db, storage
import vertexai
from vertexai.preview.vision_models import Image, ImageTextModel
import os

# Configuración de Firebase
FIREBASE_DATABASE_URL = "https://hackaton-709f6-default-rtdb.firebaseio.com/"
FIREBASE_STORAGE_BUCKET = "hackaton-709f6.appspot.com"  # Nombre de tu bucket

# Inicializa Firebase Admin SDK
cred = credentials.Certificate("C:\\Users\\Ceszmaky\\Downloads\\fit-parity-425120-s4-59d5eab06f43.json")  # Tu archivo JSON de credenciales
firebase_admin.initialize_app(cred, {
    'databaseURL': FIREBASE_DATABASE_URL,
    'storageBucket': FIREBASE_STORAGE_BUCKET
})

# Función para descargar la imagen de Firebase Storage
def download_image_from_firebase(image_name):
    bucket = storage.bucket()
    blob = bucket.blob(image_name)
    local_file_path = os.path.join(os.getcwd(), image_name)  # Guardar en el directorio actual
    blob.download_to_filename(local_file_path)
    print(f"Imagen descargada: {local_file_path}")
    return local_file_path

# Función para obtener descripciones de imágenes usando Vertex AI
def get_short_form_image_captions(project_id: str, location: str, input_file: str) -> list:
    vertexai.init(project=project_id, location=location)

    model = ImageTextModel.from_pretrained("imagetext@001")
    source_img = Image.load_from_file(input_file)

    captions = model.get_captions(
        image=source_img,
        language="en",
        number_of_results=1,
    )

    return [caption.text for caption in captions.captions]  # Ajusta esto para obtener el texto de las descripciones

# Función para guardar el análisis en Firebase Database
def save_analysis_to_firebase(image_url, descriptions):
    ref = db.reference('/image_analysis')
    ref.push({
        'image_url': image_url,
        'descriptions': descriptions
    })
    print("Resultados guardados en Firebase.")

# Función principal para procesar la imagen
def process_image(image_name, project_id, location):
    # Descargar la imagen de Firebase
    image_path = download_image_from_firebase(image_name)
    
    # Obtener descripciones usando Vertex AI
    descriptions = get_short_form_image_captions(project_id, location, image_path)
    
    # Imprimir las descripciones en la terminal
    print(f"Descripciones de la imagen: {descriptions}")

    # Guardar resultados en Firebase Database
    save_analysis_to_firebase(image_name, descriptions)

# Ejemplo de uso: Procesar la imagen desde Firebase
if __name__ == "__main__":
    project_id = "fit-parity-425120-s4"  # ID de tu proyecto
    location = "us-central1"  # Región de tu proyecto
    image_name = "agrobot.jpg"  # Nombre de la imagen en Firebase
    process_image(image_name, project_id, location)
