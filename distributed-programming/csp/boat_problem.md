# Boat crossing problem 

On one bank of a river there is a boat that ferries passengers to the other bank, and it holds exactly ten passengers.  
The boat is used by men, women and children. The boat may cast off only when it holds exactly as many passengers as its capacity,  
and only on the condition that at least two men are aboard. Children may not board unless at least one adult is already in the boat,  
and when the crossing is over, children must not be left alone in the boat. Assume that once all passengers have disembarked,  
the boat is immediately ready to take on the next group.  

Use the CSP (Communicating Sequential Processes) to solve this problem.

## Solution in CSP

    MAN :: [
      *[
      BOAT!waiting();
      BOAT?board();

      CROOS_RIVER;

      BOAT!leave();
      BOAT?unboard();
      ]
    ]

    BOAT :: [
      man, woman, child, passenger:integer;
      *[
        man := 0; woman := 0; child := 0, passenger := 0;
        *[
          passenger<10; MAN?waiting() -> [
            man = man + 1;
            passenger = passenger + 1;
            MAN!board();
          ]
          [] passenger < 10; man >= 2; WOMAN?waiting() -> [
            woman = woman + 1;
            passenger = passenger + 1;
            WOMAN!board();
          ]
          [] passenger < 10; man >= 2; CHILD?waiting() -> [
            child = child + 1;
            passenger = passenger + 1;
            CHILD!board();
          ]
        ]

        CROOS_RIVER;

        *[
          passenger > 0; CHILD?leave() -> [
            child = child - 1;
            passenger = passenger - 1;
            CHILD!unboard();
          ]
          [] passenger > 0; child = 0; WOMAN?leave() -> [
            woman = woman - 1;
            passenger = passenger - 1;
            WOMAN!unboard();
          ]
          [] passenger > 0; child = 0; MAN?leave() -> [
            man = man - 1;
            passenger = passenger - 1;
            MAN!unboard();
          ]
        ]
      ]
    ]
