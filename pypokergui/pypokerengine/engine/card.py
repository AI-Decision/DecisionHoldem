class Card:

  CLUB = 2
  DIAMOND = 4
  HEART = 8
  SPADE = 16

  SUIT_MAP = {
      2  : 'S',
      4  : 'H',
      8  : 'D',
      16 : 'C'
  }

  RANK_MAP = {
      2  :  '2',
      3  :  '3',
      4  :  '4',
      5  :  '5',
      6  :  '6',
      7  :  '7',
      8  :  '8',
      9  :  '9',
      10 : 'T',
      11 : 'J',
      12 : 'Q',
      13 : 'K',
      14 : 'A'
  }


  def __init__(self, suit, rank):
    self.suit = suit
    self.rank = 14 if rank == 1 else rank

  def __eq__(self, other):
    return self.suit == other.suit and self.rank == other.rank

  def __str__(self):
    suit = self.SUIT_MAP[self.suit]
    rank = self.RANK_MAP[self.rank]
    return "{0}{1}".format(suit, rank)

  def to_id(self):
    rank = 1 if self.rank == 14 else self.rank
    num = 0
    tmp = self.suit >> 1
    while tmp&1 != 1:
      num += 1
      tmp >>= 1

    return rank + 13 * num

  @classmethod
  def from_id(cls, card_id):
    suit, rank = 2, card_id
    while rank > 13:
      suit <<= 1
      rank -= 13

    return cls(suit, rank)

  @classmethod
  def from_str(cls, str_card):
    assert(len(str_card)==2)
    inverse = lambda hsh: {v:k for k,v in hsh.items()}
    suit = inverse(cls.SUIT_MAP)[str_card[0].upper()]
    rank = inverse(cls.RANK_MAP)[str_card[1]]
    return cls(suit, rank)
