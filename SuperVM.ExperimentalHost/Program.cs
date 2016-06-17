using ICSharpCode.SharpZipLib.Core;
using ICSharpCode.SharpZipLib.Zip;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SuperVM.ExperimentalHost
{
	class Program
	{
		private static bool running;

		static void Main(string[] args)
		{
			var zf = new ZipFile("executable.zip");

			NameValueCollection meta;
			var metaConfFile = zf.GetEntry("meta.conf");
			using (var stream = zf.GetInputStream(metaConfFile))
			{
				meta = ReadConfiguration(stream);
			}

			// Default memory size to
			int memsize = int.Parse(meta["memsize"] ?? "16384");

			var codeName = meta["code"] ?? "code.bin";
			var dataName = meta["data"];

			if (codeName == null)
				throw new InvalidDataException();

			Module module;
			{ // Load code
				var codeFile = zf.GetEntry(codeName);
				var bits = new byte[codeFile.Size];
				using (var stream = zf.GetInputStream(codeFile))
				{
					for (int i = 0; i < bits.Length; i += stream.Read(bits, i, bits.Length - i)) ;
				}
				var instr = new ulong[bits.Length / sizeof(ulong)];
				Buffer.BlockCopy(bits, 0, instr, 0, instr.Length * sizeof(ulong));

				module = new Module(instr.Select(Instruction.Decode).ToArray());
			}

			byte[] initialMemory = null;
			if (dataName != null)
			{
				var dataFile = zf.GetEntry(dataName);
				using (var stream = zf.GetInputStream(dataFile))
				{
					using (var ms = new MemoryStream())
					{
						stream.CopyTo(ms);
						initialMemory = ms.ToArray();
					}
				}
			}

			Memory memory = new Memory(memsize);
			if (initialMemory != null)
			{
				Array.Copy(initialMemory, memory.Raw, Math.Min(initialMemory.Length, memsize));
			}

			var process = new Process(module, memory);
			process.SysCall += Process_SysCall;

			running = true;
			var watch = Stopwatch.StartNew();
			long instructions = 0;
			while (running)
			{
				process.Step();
				instructions++;
			}
			watch.Stop();

			Console.WriteLine();
			Console.WriteLine("Time:  {0}", watch.Elapsed);
			Console.WriteLine("Count: {0}", instructions);
			Console.WriteLine("IPS:   {0}", instructions / watch.Elapsed.TotalSeconds);

			Console.ReadLine();
		}

		private static void Process_SysCall(object sender, Process.CommandExecutionEnvironment e)
		{
			switch (e.Additional)
			{
				case 0: running = false; break;
				case 1: // putc
					Console.Write((char)(e.Input0 & 0xFF));
					break;
				case 2: // puti
					Console.Write(e.Input0);
					break;
				default:
					throw new InvalidOperationException("Unkown syscall: " + e.Additional);
			}
		}

		static NameValueCollection ReadConfiguration(Stream stream)
		{
			var nvc = new NameValueCollection();
			using (var reader = new StreamReader(stream, Encoding.ASCII))
			{
				while (!reader.EndOfStream)
				{
					var line = reader.ReadLine();
					if (line.StartsWith("#"))
						continue;
					var parts = line.Split('=');
					if (parts.Length != 2)
						throw new InvalidDataException();
					nvc[parts[0].Trim()] = parts[1].Trim();
				}
			}
			return nvc;
		}

		// Calling example:
		// WebClient webClient = new WebClient();
		// 	Stream data = webClient.OpenRead("http://www.example.com/test.zip");
		// This stream cannot be opened with the ZipFile class because CanSeek is false.
		// UnzipFromStream(data, @"c:\temp");

		public void UnzipFromStream(Stream zipStream, string outFolder)
		{
			ZipInputStream zipInputStream = new ZipInputStream(zipStream);
			ZipEntry zipEntry = zipInputStream.GetNextEntry();
			while (zipEntry != null)
			{
				String entryFileName = zipEntry.Name;
				// to remove the folder from the entry:- entryFileName = Path.GetFileName(entryFileName);
				// Optionally match entrynames against a selection list here to skip as desired.
				// The unpacked length is available in the zipEntry.Size property.

				byte[] buffer = new byte[4096];     // 4K is optimum

				// Manipulate the output filename here as desired.
				String fullZipToPath = Path.Combine(outFolder, entryFileName);
				string directoryName = Path.GetDirectoryName(fullZipToPath);
				if (directoryName.Length > 0)
					Directory.CreateDirectory(directoryName);

				// Unzip file in buffered chunks. This is just as fast as unpacking to a buffer the full size
				// of the file, but does not waste memory.
				// The "using" will close the stream even if an exception occurs.
				using (FileStream streamWriter = File.Create(fullZipToPath))
				{
					StreamUtils.Copy(zipInputStream, streamWriter, buffer);
				}
				zipEntry = zipInputStream.GetNextEntry();
			}
		}


		public void ExtractZipFile(string archiveFilenameIn, string password, string outFolder)
		{
			ZipFile zf = null;
			try
			{
				FileStream fs = File.OpenRead(archiveFilenameIn);
				zf = new ZipFile(fs);
				if (!String.IsNullOrEmpty(password))
				{
					zf.Password = password;     // AES encrypted entries are handled automatically
				}
				foreach (ZipEntry zipEntry in zf)
				{
					if (!zipEntry.IsFile)
					{
						continue;           // Ignore directories
					}
					String entryFileName = zipEntry.Name;
					// to remove the folder from the entry:- entryFileName = Path.GetFileName(entryFileName);
					// Optionally match entrynames against a selection list here to skip as desired.
					// The unpacked length is available in the zipEntry.Size property.

					byte[] buffer = new byte[4096];     // 4K is optimum
					Stream zipStream = zf.GetInputStream(zipEntry);

					// Manipulate the output filename here as desired.
					String fullZipToPath = Path.Combine(outFolder, entryFileName);
					string directoryName = Path.GetDirectoryName(fullZipToPath);
					if (directoryName.Length > 0)
						Directory.CreateDirectory(directoryName);

					// Unzip file in buffered chunks. This is just as fast as unpacking to a buffer the full size
					// of the file, but does not waste memory.
					// The "using" will close the stream even if an exception occurs.
					using (FileStream streamWriter = File.Create(fullZipToPath))
					{
						StreamUtils.Copy(zipStream, streamWriter, buffer);
					}
				}
			}
			finally
			{
				if (zf != null)
				{
					zf.IsStreamOwner = true; // Makes close also shut the underlying stream
					zf.Close(); // Ensure we release resources
				}
			}
		}
	}
}
