       *> { dg-do run }

       IDENTIFICATION   DIVISION.
       PROGRAM-ID.      prog.
       DATA             DIVISION.
       WORKING-STORAGE  SECTION.
       01  R            PIC S9(4)V9(4) COMP-5 VALUE 4.1.
       01  Z            PIC 9 COMP-5 VALUE 0.
       PROCEDURE        DIVISION.
           MOVE FUNCTION REM ( -11 Z ) TO R
           IF FUNCTION EXCEPTION-STATUS
           NOT = 'EC-ARGUMENT-FUNCTION'
              DISPLAY 'Wrong/missing exception: '
                      FUNCTION EXCEPTION-STATUS
              END-DISPLAY
           END-IF
           IF R NOT = 0
              DISPLAY 'result is not zero: ' R
              END-DISPLAY
           END-IF
           STOP RUN.

