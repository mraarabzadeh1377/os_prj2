struct ticketlock
{
    int turn;
    int lastTicketNumber;

    // For debugging:
    char *name; // Name of lock.
};
