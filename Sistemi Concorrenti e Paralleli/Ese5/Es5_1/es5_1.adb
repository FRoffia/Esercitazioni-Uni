--------------------------------------------------------------------------------
--  *  Prog name ponte.adb
--  *  Project name ponte
--Es5 del 13 maggio 2024 - PONTE A SENSO UNICO ALTERNATO
--  *
--------------------------------------------------------------------------------

with Ada.Text_IO, Ada.Integer_Text_IO;
use Ada.Text_IO, Ada.Integer_Text_IO;

procedure es5_1 is
   N : constant Integer := 3;
   N_C : constant Integer := 10;

   type cliente_ID is range 1..N_C;			-- 10 clienti
   type sportello_ID is range 1..N;			-- N sportelli

   type req_ID is  (TUR, EVE); 	-- tipo di richieste, turistiche o per eventi

   task type cliente (ID: cliente_ID; REQ: req_ID);		-- task che rappresenta il generico cliente, con id cliente e tipo di richiesta
   type ac is access cliente;				-- riferimento ad un task cliente
   

   -- processo gestore del pool
   task type server is
      entry infoTUR (cl_ID: in cliente_ID );
      entry infoEVE(cl_ID: in cliente_ID );
	   entry rilascio (cl_ID: in cliente_ID );
   end server;

   S : server;

   ------ definizione PROCESSO server:
   task body server is

   sportelli_pieni: array(sportello_ID'Range) of Integer;--mi segno quali sportelli sono pieni
   pieni: Integer := 0;
   gestiti : Integer := 0;

   begin

   Put_Line ("Server iniziato!");

   --INIZIALIZZAZIONI:
	for i in sportello_ID'Range loop
      sportelli_pieni(i):=0;
   end loop;

   --Gestione richieste
   while gestiti < N_C loop
		select
         when pieni < sportello_ID'Range_Length  => accept infoTUR (cl_ID: in cliente_ID ) do --possibile l'accesso per info TUR
            --ricerca del primo sportello libero
            spor_ID : Integer :=0;
            for i in sportello_ID'Range loop
               if sportelli_pieni(i) = 0 then
                  sportelli_pieni(i) := Integer(cl_ID);
                  spor_ID := Integer(i);
                  pieni:=pieni+1;
                  exit;
               end if;
            end loop;

            Put_Line("S"&spor_ID'Image&": accettato il cliente "& cliente_ID'Image(cl_ID) &" per info TUR.");
            Put_Line("Ora ci sono "& pieni'Image &" sportelli pieni.");
            New_Line;
         end infoTUR; -- end of the synchronised part

         or when pieni < sportello_ID'Range_Length and infoTUR'Count = 0  => accept infoEVE (cl_ID: in cliente_ID ) do -- possibile l'accesso per info EVE
            --ricerca del primo sportello libero
            spor_ID : Integer :=0;
            for i in sportello_ID'Range loop
               if sportelli_pieni(i) = 0 then
                  sportelli_pieni(i) := Integer(cl_ID);
                  spor_ID := Integer(i);
                  pieni:=pieni+1;
                  exit;
               end if;
            end loop;

            Put_Line("S"&spor_ID'Image&": accettato il cliente "& cliente_ID'Image(cl_ID) &" per info EVE.");
            Put_Line("Ora ci sono "& pieni'Image &" sportelli pieni.");
            New_Line;
         end infoEVE;               -- end of the synchronised part

         or accept rilascio (cl_ID: in cliente_ID ) do
            --ricerca dell'id sportello corrispondente
            spor_ID : Integer :=0;
            for i in sportello_ID'Range loop
               if sportelli_pieni(i) = Integer(cl_ID) then
                  sportelli_pieni(i) := 0;
                  spor_ID := Integer(i);
                  pieni:=pieni-1;
                  gestiti:=gestiti+1;
                  exit;
               end if;
            end loop;
            Put_Line("S"&spor_ID'Image&": rilasciato il cliente "& cliente_ID'Image(cl_ID) &".");
            Put_Line("Ora ci sono "& pieni'Image &" sportelli pieni.");
            New_Line;
         end rilascio;               -- end of the synchronised part
      end select;
   end loop;
   end;--end task server

 ------------------processo cliente: usa il ponte
   task body cliente is
   begin
      Put_Line ("Cliente" & cliente_ID'Image (ID) & " iniziato!");
      if REQ = TUR then
         S.infoTUR(ID);
         delay 1.0;
         S.rilascio(ID);
      else
         S.infoEVE(ID);
         delay 1.0;
         S.rilascio(ID);
      end if;
   end;
------------------------------- main:
   New_client: ac;
   

   begin -- equivale al main

      for I in cliente_ID'Range loop  -- ciclo creazione task
         if Integer(I) mod 2 = 0 then
            New_client := new cliente (I,TUR); -- creazione cliente I-simo
         else 
            New_client := new cliente (I,EVE); -- creazione cliente I-simo
         end if;
      end loop;
end es5_1;
