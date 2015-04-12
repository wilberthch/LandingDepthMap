using Microsoft.Azure.Documents;
using Microsoft.Azure.Documents.Client;
using Microsoft.Azure.Documents.Linq;
using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Threading.Tasks;
using System.Threading;

namespace ConsoleApplication1
{

    class Program
    {
        internal sealed class Imagenes
        {
            [JsonProperty(PropertyName = "id")]
            public string imageDump { get; set; }
            public string DataMatrix;
            
        }
        private static string EndpointUrl = "https://landing-storage.documents.azure.com:443/";
        private static string AuthorizationKey = "pXtmQCIaDPnTV0CzNML3+gBykBX7ZYzDE19U1gOhVNpTeCo+qPyRYFS5GJbmJQ5SLeC8dvdFWzn0RZR/OGLbZQ==";
        private static DocumentClient client = new DocumentClient(new Uri(EndpointUrl), AuthorizationKey);

        private static async Task GetStartedDemo(int numeroArchivo)
        {
            // Create a new instance of the DocumentClient.
            
            //Get, or Create, the Database

            string fileName = "C:\\Users\\cic\\Downloads\\2_Depth\\2_Depth\\" + (numeroArchivo).ToString() + ".txt";
            string fileNameIndex = "C:\\Users\\cic\\Downloads\\2_Depth\\2_Depth\\" + (numeroArchivo).ToString() + "-index.txt";
            System.IO.StreamReader objReader;
            objReader = new System.IO.StreamReader(fileName);
            string datos1 = objReader.ReadToEnd();
            objReader.Close();
            System.IO.StreamReader objReader2;
            objReader2 = new System.IO.StreamReader(fileNameIndex);
            string datos2 = objReader2.ReadToEnd();
            objReader2.Close();
            Database database = await GetOrCreateDatabaseAsync("LandingData");
            Imagenes actual = new Imagenes
            {
                DataMatrix = datos1,
                imageDump = datos2


            };
            //Get, or Create, the Document Collection
            DocumentCollection collection = await GetOrCreateCollectionAsync(database.SelfLink, "LandingDoc");
            await client.CreateDocumentAsync(collection.DocumentsLink, actual);
            
            

        }




        static void Main(string[] args)
        {
            bool entra =false;
            //Extraer nombre de archivos 
            string startFolder = @"C:\Users\cic\Downloads\2_Depth\2_Depth\";

            // Take a snapshot of the file system.
            System.IO.DirectoryInfo dir = new System.IO.DirectoryInfo(startFolder);

            // This method assumes that the application has discovery permissions
            // for all folders under the specified path.
            IEnumerable<System.IO.FileInfo> fileList = dir.GetFiles("*.*", System.IO.SearchOption.TopDirectoryOnly);

            //Create the query
            IEnumerable<System.IO.FileInfo> fileQuery =
                from file in fileList
                where file.Extension == ".txt"
                orderby file.Name
                select file;

            //Execute the query. This might write out a lot of files!
            int cantidadArchivos=fileQuery.Count();
            foreach (System.IO.FileInfo fi in fileQuery)
            {
                Console.WriteLine(fi.FullName);
            }

            // Create and execute a new query by using the previous 
            // query as a starting point. fileQuery is not 
            // executed again until the call to Last()
            var newestFile =
                (from file in fileQuery
                 orderby file.CreationTime
                 select new { file.FullName, file.CreationTime })
                .Last();

            Console.WriteLine("\r\nThe newest .txt file is {0}. Creation time: {1}",
                newestFile.FullName, newestFile.CreationTime);

            
            for (int i = 0; i < cantidadArchivos/2; i++)
            {

                
                try
                {
                    GetStartedDemo(i).Wait();
                }
                catch (Exception e)
                {
                    Exception baseException = e.GetBaseException();
                    Console.WriteLine("Error: {0}, Message: {1}", e.Message, baseException.Message);
                }
                Thread.Sleep(5000);

            }
        }

        private static async Task<DocumentCollection> GetOrCreateCollectionAsync(string dbLink, string id)
        {
            DocumentCollection collection = client.CreateDocumentCollectionQuery(dbLink).Where(c => c.Id == id).ToArray().FirstOrDefault();
            if (collection == null)
            {
                collection = await client.CreateDocumentCollectionAsync(dbLink, new DocumentCollection { Id = id });
            }

            return collection;
        }

        /// <summary>
        /// Get a Database by id, or create a new one if one with the id provided doesn't exist.
        /// </summary>
        /// <param name="id">The id of the Database to search for, or create.</param>
        /// <returns>The matched, or created, Database object</returns>
        private static async Task<Database> GetOrCreateDatabaseAsync(string id)
        {
            Database database = client.CreateDatabaseQuery().Where(db => db.Id == id).ToArray().FirstOrDefault();
            if (database == null)
            {
                database = await client.CreateDatabaseAsync(new Database { Id = id });
            }

            return database;
        }

    }
}
