#ifndef GAMES_H
#define GAMES_H

#include <QWidget>

/**
 * @author Carsten Niehaus
 * @brief A token to play a game
 */
class Stone : public QObject
{
	Q_OBJECT

	public:
		enum PLAYER
		{
			Black = 0,
			White
		};

		/**
		 * if the stone is black make it white and vice versa
		 */
		void swap();
		
		/**
		 * Set the kind of player to @p player
		 */
		Stone( PLAYER player, const QPoint& point );

		/**
		 * set the player to @p player
		 */
		void setPlayer( PLAYER player )
		{
			m_player = player;
		}

		/**
		 * @return the player of this stone
		 */
		PLAYER player() const
		{
			return m_player;
		}

		/**
		 * @return the position of the stone
		 */
		QPoint position() const
		{
			return m_position;
		}

		/**
		 * set the position to @p point
		 */
		void setPosition( const QPoint& point )
		{
			m_position = point;
			emit moved( m_position );
		}

	private:
		PLAYER m_player;

		QPoint m_position;

	signals:
		void removed();
		void moved(QPoint);
};

/**
 * @author Carsten Niehaus
 * @brief The playground of a game
 */
class Field
{
	public:
		/**
		 * Constructor
		 *
		 * @param x The x-size of the field
		 * @param y The y-size of the field
		 */
		Field(int x, int y);

		/**
		 * @return the x-size of the field
		 */
		int xSize() const
		{
			return m_size_x;
		}
		
		/**
		 * @return the y-size of the field
		 */
		int ySize() const
		{
			return m_size_y;
		}
		
		/**
		 * Destructor
		 */
		virtual ~Field();

		/**
		 * Add the Stone @p stone to position @p pos
		 */
		virtual void addStone( Stone* stone ) = 0;

		/**
		 * @return the Stone at the position @p pos
		 */
		virtual Stone* stoneAtPosition( const QPoint& pos );

		/**
		 * moves Stone @p stone from the current position to the @p newPosition
		 */
		virtual void moveStoneTo( Stone* stone, const QPoint& newPosition ) = 0;
		
		/**
		 * moves the Stone in @p currentPosition to the @p newPosition
		 */
		virtual void moveStoneTo( const QPoint& currentPosition, const QPoint& newPosition )
		{
			Stone* s = stoneAtPosition( currentPosition );
			moveStoneTo( s, newPosition );
		}
		
		/**
		 * @return the stones of the game
		 */
		virtual QList<Stone*> stones() const
		{
			return m_stones;
		}
		
	protected:
		int m_size_x;
		
		int m_size_y;
		
		QList<Stone*> m_stones;
};


/**
 * @author Carsten Niehaus
 * @brief Baseclass for all games
 */
class Game
{
 	public:
		/**
		 * Destructor
		 */
		virtual ~Game();

		/**
		 * starts the game
		 */
		virtual void startGame();

		virtual void rollDices() = 0;
		
		/**
		 * stops the game
		 */
		virtual void stopGame();

		/**
		 * set the field to @p field
		 */
		void setField( Field* field )
		{
			m_field = field;
		}

	protected:
		Field* m_field;

		/**
		 * The constructor
		 */
		Game();
};

/**
 * @author Carsten Niehaus
 */
class RAGame : public Game
{
	public:
		RAGame();

		void rollDices();
		
		class RAField : public Field
		{
			public:
				/**
				 * Constructor
				 *
				 * @param x The x-size of the field
				 * @param y The y-size of the field
				 */
				RAField(int x, int y);

				/**
				 * moves Stone @p stone from the current position to the @p newPosition
				 */
				virtual void moveStoneTo( Stone* stone, const QPoint& newPosition );
		
				virtual void addStone( Stone* stone );
		};
	private:
		int m_counter;
		int m_number;

	protected:
		RAField* m_field;
};
#endif // GAMES_H